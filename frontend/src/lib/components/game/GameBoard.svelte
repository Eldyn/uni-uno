<script lang="ts">
	import { storeGame } from "../../stores/game.svelte";
	import { untrack } from "svelte";

	let { gameColor = "green" }: { gameColor?: string } = $props();

	// ─────────────────────────────────────────────────────────────────────────
	// FLYING CARD OVERLAY
	// ─────────────────────────────────────────────────────────────────────────
	interface FlyingCard {
		id: number;
		color: string;
		value: string;
		turned: boolean;
		srcRect: DOMRect;
		dstRect: DOMRect;
		key: number;
	}

	let flyingCards = $state<FlyingCard[]>([]);
	let flyKey = $state(0);

	let drawPileEl = $state<HTMLElement | null>(null);
	let discardPileEl = $state<HTMLElement | null>(null);
	let handEl = $state<HTMLElement | null>(null);
	let opponentHandEls = $state<(HTMLElement | null)[]>([null, null, null]);

	function getRect(el: HTMLElement | null): DOMRect {
		return (
			el?.getBoundingClientRect() ??
			new DOMRect(window.innerWidth / 2, window.innerHeight / 2, 80, 120)
		);
	}

	function launchCard(card: FlyingCard) {
		flyingCards = [...flyingCards, card];
		setTimeout(() => {
			flyingCards = flyingCards.filter((c) => c.key !== card.key);
		}, 620);
	}

	// ─────────────────────────────────────────────────────────────────────────
	// CARD ORDER  (client-side only, never sent to backend)
	// ─────────────────────────────────────────────────────────────────────────
	let cardOrder = $state<Map<number, number>>(new Map());
	let draggedCardId = $state<number | null>(null);
	let dragOverCardId = $state<number | null>(null);
	let isDragging = $state(false);

	// ─────────────────────────────────────────────────────────────────────────
	// HAND TRACKING — detect drawn cards + played cards
	// ─────────────────────────────────────────────────────────────────────────
	let prevHandIds = $state<Set<number>>(new Set());
	let hiddenCardIds = $state<Set<number>>(new Set());
	let prevTopCardId = $state<number | null>(null);
	let prevOpponentCounts = $state<Map<string, number>>(new Map());
	let playableCardIds = $state<Set<number>>(new Set());

	// ─────────────────────────────────────────────────────────────────────────
	// REACTIVE HAND & SIDE EFFECTS
	// ─────────────────────────────────────────────────────────────────────────
	let hand = $derived(storeGame.localPlayer?.hand ?? []);

	$effect(() => {
		const currentHand = hand;
		const currentIds = new Set(currentHand.map((c) => c.id));

		untrack(() => {
			const newIds: number[] = [];
			for (const id of currentIds) {
				if (!prevHandIds.has(id)) newIds.push(id);
			}

			const removedIds: number[] = [];
			for (const id of prevHandIds) {
				if (!currentIds.has(id)) removedIds.push(id);
			}

			if (newIds.length > 0 || removedIds.length > 0) {
				for (const id of newIds) {
					const card = currentHand.find((c) => c.id === id);
					if (!card) continue;
					const src = getRect(drawPileEl);
					const dst = getRect(handEl);
					const k = ++flyKey;
					hiddenCardIds = new Set([...hiddenCardIds, id]);
					setTimeout(() => {
						hiddenCardIds = new Set([...hiddenCardIds].filter((x) => x !== id));
					}, 550);
					launchCard({
						id,
						color: card.color,
						value: card.value,
						turned: true,
						srcRect: src,
						dstRect: dst,
						key: k
					});
				}
			}

			prevHandIds = currentIds;

			const next = new Map(cardOrder);
			let mapChanged = false;

			for (const [id] of next) {
				if (!currentIds.has(id)) {
					next.delete(id);
					mapChanged = true;
				}
			}

			let maxOrd = next.size > 0 ? Math.max(...next.values()) : -1;
			for (const c of currentHand) {
				if (!next.has(c.id)) {
					next.set(c.id, ++maxOrd);
					mapChanged = true;
				}
			}

			if (
				mapChanged ||
				next.size !== cardOrder.size ||
				[...next].some(([k, v]) => cardOrder.get(k) !== v)
			) {
				cardOrder = next;
			}
		});
	});

	let sortedHand = $derived(
		[...hand].sort((a, b) => (cardOrder.get(a.id) ?? 0) - (cardOrder.get(b.id) ?? 0))
	);

	// ─────────────────────────────────────────────────────────────────────────
	// PLAY CARD ANIMATION
	// ─────────────────────────────────────────────────────────────────────────
	let lastKnownHand = $state<{ id: number; color: string; value: string }[]>([]);

	$effect(() => {
		const top = storeGame.state?.top_card;
		const currentHand = storeGame.localPlayer?.hand ?? [];

		untrack(() => {
			if (!top) return;

			if (top.id !== prevTopCardId) {
				const currentIds = new Set(currentHand.map((c) => c.id));
				const playedCard = lastKnownHand.find((c) => !currentIds.has(c.id) && c.id === top.id);

				if (playedCard) {
					const src = getRect(handEl);
					const dst = getRect(discardPileEl);
					const k = ++flyKey;
					launchCard({
						id: playedCard.id,
						color: playedCard.color,
						value: playedCard.value,
						turned: false,
						srcRect: src,
						dstRect: dst,
						key: k
					});
				}
				prevTopCardId = top.id;
			}
			lastKnownHand = currentHand.map((c) => ({ id: c.id, color: c.color, value: c.value }));
		});
	});

	// ─────────────────────────────────────────────────────────────────────────
	// OPPONENT DRAW ANIMATION
	// ─────────────────────────────────────────────────────────────────────────
	$effect(() => {
		const players = storeGame.state?.players ?? [];
		const localUser = storeGame.localPlayer?.username;

		untrack(() => {
			for (const p of players) {
				if (p.username === localUser) continue;
				const prev = prevOpponentCounts.get(p.username) ?? p.card_count;
				const delta = p.card_count - prev;

				if (delta > 0) {
					const opponents = players.filter((x) => x.username !== localUser);
					const idx = opponents.findIndex((x) => x.username === p.username);
					const opEl = opponentHandEls[idx] ?? null;

					const src = getRect(drawPileEl);
					const dst = getRect(opEl);

					for (let i = 0; i < delta; i++) {
						const k = ++flyKey;
						setTimeout(() => {
							launchCard({
								id: -k,
								color: "black",
								value: "",
								turned: true,
								srcRect: src,
								dstRect: dst,
								key: k
							});
						}, i * 80);
					}
				}
			}

			const next = new Map<string, number>();
			for (const p of players) {
				if (p.username !== localUser) next.set(p.username, p.card_count);
			}
			prevOpponentCounts = next;
		});
	});

	// ─────────────────────────────────────────────────────────────────────────
	// DRAG-AND-DROP  (hand reordering)
	// ─────────────────────────────────────────────────────────────────────────
	function onDragStart(e: DragEvent, cardId: number) {
		draggedCardId = cardId;
		isDragging = true;
		e.dataTransfer?.setData("text/plain", String(cardId));
	}
	function onDragOver(e: DragEvent, cardId: number) {
		e.preventDefault();
		if (draggedCardId !== cardId) dragOverCardId = cardId;
	}
	function onDragLeave() {
		dragOverCardId = null;
	}
	function onDrop(e: DragEvent, targetId: number) {
		e.preventDefault();
		if (draggedCardId === null || draggedCardId === targetId) {
			resetDrag();
			return;
		}
		const from = cardOrder.get(draggedCardId) ?? 0;
		const to = cardOrder.get(targetId) ?? 0;
		const next = new Map(cardOrder);
		if (from < to) {
			for (const [id, o] of next) {
				if (o > from && o <= to) next.set(id, o - 1);
			}
		} else {
			for (const [id, o] of next) {
				if (o < from && o >= to) next.set(id, o + 1);
			}
		}
		next.set(draggedCardId, to);
		cardOrder = next;
		resetDrag();
	}
	function onDragEnd() {
		resetDrag();
	}
	function resetDrag() {
		draggedCardId = null;
		dragOverCardId = null;
		isDragging = false;
	}

	// ─────────────────────────────────────────────────────────────────────────
	// TOUCH REORDER  (mobile)
	// ─────────────────────────────────────────────────────────────────────────
	let touchStartX = $state(0);
	let touchCardId = $state<number | null>(null);
	let touchStartIdx = $state(0);

	function onTouchStart(e: TouchEvent, cardId: number) {
		touchCardId = cardId;
		touchStartX = e.touches[0].clientX;
		touchStartIdx = cardOrder.get(cardId) ?? 0;
	}
	function onTouchEnd(e: TouchEvent) {
		if (touchCardId === null) return;
		const delta = Math.round((e.changedTouches[0].clientX - touchStartX) / 50);
		if (delta !== 0) {
			const handArr = storeGame.localPlayer?.hand ?? [];
			const newOrd = Math.max(0, Math.min(handArr.length - 1, touchStartIdx + delta));
			const target = [...cardOrder].find(([, o]) => o === newOrd);
			if (target) onDrop({ preventDefault: () => {} } as DragEvent, target[0]);
		}
		touchCardId = null;
	}

	// ─────────────────────────────────────────────────────────────────────────
	// ACTIONS & DERIVATIONS
	// ─────────────────────────────────────────────────────────────────────────
	function handleCardClick(cardId: number) {
		if (!isDragging) storeGame.playCard(cardId);
	}
	function handleDrawClick() {
		if (storeGame.state?.current_turn === storeGame.localPlayer?.username) {
			storeGame.drawCard();
		}
	}

	let opponents = $derived(
		storeGame.state?.players.filter((p) => p.username !== storeGame.localPlayer?.username) ?? []
	);
	let leftPlayer = $derived(opponents[0]);
	let topPlayer = $derived(opponents[1]);
	let rightPlayer = $derived(opponents[2]);

	function cardValueImg(value: string): string | null {
		if (value === "Skip") return "/images/skip.png";
		if (value === "Rev") return "/images/reverse.png";
		if (value === "Wild") return "/images/wild.png";
		return null;
	}
</script>

{#each flyingCards as fc (fc.key)}
	<div
		class="flying-card card {fc.turned ? 'turned' : fc.color}"
		style="
            --src-x: {fc.srcRect.left + fc.srcRect.width / 2}px;
            --src-y: {fc.srcRect.top + fc.srcRect.height / 2}px;
            --dst-x: {fc.dstRect.left + fc.dstRect.width / 2}px;
            --dst-y: {fc.dstRect.top + fc.dstRect.height / 2}px;
            left: var(--src-x); top: var(--src-y);
        "
	>
		<div class="bckg"></div>
		{#if !fc.turned}
			{@const img = cardValueImg(fc.value)}
			<div class="card-value">
				{#if img}
					<img src={img} alt={fc.value} class="card-icon" />
				{:else}
					<span class="text-value" class:small-text={fc.value.length > 1}>{fc.value}</span>
				{/if}
			</div>
		{/if}
	</div>
{/each}

<div class="sfondo-gioco"></div>

<div class="game-field perspective {gameColor}">
	<div id="player" style="position: relative; width: 100%; height: 100%;">
		<div class="player-label">(You) {storeGame.localPlayer?.username || ""}</div>
		<div
			bind:this={handEl}
			class="player_hand"
			style="
                position: absolute; top: 50%; left: 50%;
                transform: translate(-50%, -30%);
                width: calc({sortedHand.length} * 2.2em + 7.2em);
                height: calc(var(--cardSize) * 1.5357);
            "
		>
			{#each sortedHand as card, i (card.id)}
				{@const isHidden = hiddenCardIds.has(card.id)}
				{@const isDragged = draggedCardId === card.id}
				{@const isDragTgt = dragOverCardId === card.id}
				{@const isPlayable = playableCardIds.has(card.id)}
				{@const imgSrc = cardValueImg(card.value)}

				<div
					class="card {card.color}"
					class:card--playable={isPlayable}
					class:card--dragging={isDragged}
					class:card--drag-target={isDragTgt}
					class:card--hidden={isHidden}
					role="button"
					tabindex="0"
					draggable="true"
					style="left: calc({i} * 2.2em + 1.1em)"
					onclick={() => handleCardClick(card.id)}
					onkeydown={(e) => e.key === "Enter" && handleCardClick(card.id)}
					ondragstart={(e) => onDragStart(e, card.id)}
					ondragover={(e) => onDragOver(e, card.id)}
					ondragleave={onDragLeave}
					ondrop={(e) => onDrop(e, card.id)}
					ondragend={onDragEnd}
					ontouchstart={(e) => onTouchStart(e, card.id)}
					ontouchend={onTouchEnd}
				>
					<div class="bckg"></div>
					<div class="card-value">
						{#if imgSrc}
							<img src={imgSrc} alt={card.value} class="card-icon" />
						{:else}
							<span class="text-value" class:small-text={card.value.length > 1}>{card.value}</span>
						{/if}
					</div>
					{#if isPlayable}
						<div class="playable-glow"></div>
					{/if}
				</div>
			{/each}
		</div>
	</div>

	<div id="player_left" style="position: relative; width: 100%; height: 100%;">
		<div class="player-label">{leftPlayer ? leftPlayer.username : "Waiting..."}</div>
		<div
			bind:this={opponentHandEls[0]}
			class="player_hand"
			style="
                position: absolute; top: 45%; left: 50%;
                transform: translate(-50%, -50%) rotate(90deg);
                width: calc({leftPlayer?.card_count ?? 0} * 2.2em + 7.2em);
                height: calc(var(--cardSize) * 1.5357);
            "
		>
			{#each Array(leftPlayer?.card_count ?? 0) as _, n}
				<div class="card turned" style="left: calc({n} * 2.2em + 1.1em)">
					<div class="bckg"></div>
				</div>
			{/each}
		</div>
	</div>

	<div id="player_top" style="position: relative; width: 100%; height: 100%;">
		<div class="player-label">{topPlayer ? topPlayer.username : "Waiting..."}</div>
		<div
			bind:this={opponentHandEls[1]}
			class="player_hand"
			style="
                position: absolute; top: 50%; left: 50%;
                transform: translate(-50%, -50%) scaleY(-1);
                width: calc({topPlayer?.card_count ?? 0} * 2.2em + 7.2em);
                height: calc(var(--cardSize) * 1.5357);
            "
		>
			{#each Array(topPlayer?.card_count ?? 0) as _, n}
				<div class="card turned" style="left: calc({n} * 2.2em + 1.1em)">
					<div class="bckg"></div>
				</div>
			{/each}
		</div>
	</div>

	<div id="player_right" style="position: relative; width: 100%; height: 100%;">
		<div class="player-label">{rightPlayer ? rightPlayer.username : "Waiting..."}</div>
		<div
			bind:this={opponentHandEls[2]}
			class="player_hand"
			style="
                position: absolute; top: 45%; left: 50%;
                transform: translate(-50%, -50%) rotate(-90deg);
                width: calc({rightPlayer?.card_count ?? 0} * 2.2em + 7.2em);
                height: calc(var(--cardSize) * 1.5357);
            "
		>
			{#each Array(rightPlayer?.card_count ?? 0) as _, n}
				<div class="card turned" style="left: calc({n} * 2.2em + 1.1em)">
					<div class="bckg"></div>
				</div>
			{/each}
		</div>
	</div>

	<div id="piles_area">
		<div id="draw_pile" bind:this={drawPileEl} onclick={handleDrawClick} role="button" tabindex="0">
			<div class="card turned top-card"><div class="bckg"></div></div>
			<div class="card turned pile"><div class="bckg"></div></div>
		</div>

		<div id="discard_pile" bind:this={discardPileEl}>
			{#if storeGame.state?.top_card}
				{@const img = cardValueImg(storeGame.state.top_card.value)}
				{#key storeGame.state.top_card.id}
					<div class="card top-card {storeGame.state.top_card.color}">
						<div class="bckg"></div>
						<div class="card-value" style="z-index:10;">
							{#if img}
								<img src={img} alt={storeGame.state.top_card.value} class="card-icon" />
							{:else}
								<span
									class="text-value"
									class:small-text={storeGame.state.top_card.value.length > 1}
								>
									{storeGame.state.top_card.value}
								</span>
							{/if}
						</div>
					</div>
				{/key}
			{:else}
				<div class="card top-card {gameColor}"><div class="bckg"></div></div>
			{/if}
			<div class="card pile"><div class="bckg"></div></div>
		</div>
	</div>
</div>

<style>
	.flying-card {
		position: fixed;
		transform: translate(-50%, -50%);
		z-index: 9999;
		pointer-events: none;
		animation: fly-to-dest 0.55s cubic-bezier(0.22, 1, 0.36, 1) forwards;
		will-change: transform, top, left;
	}

	@keyframes fly-to-dest {
		0% {
			left: var(--src-x);
			top: var(--src-y);
			transform: translate(-50%, -50%) scale(1) rotate(0deg);
			opacity: 1;
		}
		50% {
			left: calc((var(--src-x) + var(--dst-x)) / 2);
			top: calc((var(--src-y) + var(--dst-y)) / 2 - 60px);
			transform: translate(-50%, -50%) scale(1.1) rotate(8deg);
			opacity: 1;
		}
		100% {
			left: var(--dst-x);
			top: var(--dst-y);
			transform: translate(-50%, -50%) scale(0.85) rotate(0deg);
			opacity: 0;
		}
	}

	.sfondo-gioco {
		position: fixed;
		top: 0;
		left: 0;
		width: 100vw;
		height: 100vh;
		background-image: url("/images/background_red_dark.png");
		background-size: cover;
		background-position: center;
		background-repeat: no-repeat;
		z-index: 1;
		pointer-events: none;
	}

	:global(body) {
		margin: 0;
		padding: 0;
		overflow: hidden;
		background-color: transparent;
	}

	:root {
		--cardSize: 5em;
		--redCard: #dc251c;
		--yellowCard: #fcf604;
		--blueCard: #0493de;
		--greenCard: #018d41;
		--blackCard: #1f1b18;
		--lowShadow: 0 1px 3px rgba(0, 0, 0, 0.12), 0 1px 2px rgba(0, 0, 0, 0.24);
		--lowShadowHover: 0 3px 6px rgba(0, 0, 0, 0.16), 0 3px 6px rgba(0, 0, 0, 0.23);
		--shadowColor: rgba(0, 0, 0, 0.16);
		--fieldSize: 24em;
		--playerSpace: 12em;
	}

	.game-field {
		position: relative;
		z-index: 2;
		height: 100vh;
		display: grid;
		justify-content: center;
		align-content: center;
		grid-gap: 0.5em;
		grid-template-columns: var(--playerSpace) var(--fieldSize) var(--playerSpace);
		grid-template-rows: var(--playerSpace) var(--fieldSize) var(--playerSpace);
	}

	.game-field.perspective {
		transform: perspective(1200px) rotateX(10deg);
	}

	#piles_area {
		position: relative;
		border-radius: 4em;
		transition: background-color 300ms;
	}

	.game-field.yellow #piles_area {
		background-color: rgba(252, 246, 4, 0.3);
	}
	.game-field.blue #piles_area {
		background-color: rgba(4, 147, 222, 0.3);
	}
	.game-field.red #piles_area {
		background-color: rgba(220, 37, 28, 0.3);
	}
	.game-field.green #piles_area {
		background-color: rgba(1, 141, 65, 0.3);
	}

	#piles_area {
		grid-area: 2 / 2;
		transform: translateY(-2.8em);
	}
	#player_top {
		grid-area: 1 / 2;
	}
	#player_left {
		grid-area: 2 / 1;
		transform: translateX(-4.5em);
	}
	#player_right {
		grid-area: 2 / 3;
		transform: translateX(4.5em);
	}
	#player {
		grid-area: 3 / 2;
	}

	.player-label {
		position: absolute;
		font-weight: bold;
		color: white;
		text-shadow: 0 1px 4px rgba(0, 0, 0, 0.8);
		font-size: 0.95em;
		letter-spacing: 0.04em;
		white-space: nowrap;
		z-index: 110;
	}
	
    /* Posizione etichette giocatori */
	#player .player-label {
		top: -7em;
		left: 50%;
		transform: translateX(-50%);
	}

	#player_top .player-label {
		bottom: -1em;
		left: 50%;
		transform: translateX(-50%);
	}

	#player_left .player-label {
		top: 40%;
		right: -4.5em;
		transform: translateY(-50%);
	}

	#player_right .player-label {
		top: 40%;
		left: -4.5em;
		transform: translateY(-50%);
	}
    /* --- */
    
	.card {
		display: inline-block;
		background-color: white;
		border: 1px solid #ccc;
		border-radius: 0.8em;
		padding: 0.3em;
		box-shadow: var(--lowShadow);
		transition:
			transform 200ms ease,
			box-shadow 200ms ease,
			filter 200ms ease;
		position: absolute;
		will-change: transform;
	}

	.card .bckg {
		width: var(--cardSize);
		height: calc(var(--cardSize) * 1.5357);
		border-radius: 0.5em;
		overflow: hidden;
		position: relative;
	}

	.card .bckg::before {
		content: "";
		width: var(--cardSize);
		height: calc(var(--cardSize) * 1.3);
		background-color: white;
		position: absolute;
		left: 50%;
		top: 50%;
		transform: translate(-50%, -50%) rotate(10deg);
		transform-origin: center center;
		border-radius: 90% 40%;
	}

	.card.red {
		color: var(--redCard);
	}
	.card.red .bckg {
		background-color: var(--redCard);
	}
	.card.yellow {
		color: var(--yellowCard);
	}
	.card.yellow .bckg {
		background-color: var(--yellowCard);
	}
	.card.blue {
		color: var(--blueCard);
	}
	.card.blue .bckg {
		background-color: var(--blueCard);
	}
	.card.green {
		color: var(--greenCard);
	}
	.card.green .bckg {
		background-color: var(--greenCard);
	}
	.card.black {
		color: var(--blackCard);
	}
	.card.black .bckg {
		background-color: var(--blackCard);
	}
	.card.turned .bckg {
		background-color: var(--blackCard);
	}
	.card.turned .bckg::before {
		background-color: var(--redCard);
	}
	.card.turned:hover {
		cursor: default;
	}

	.card-value {
		position: absolute;
		top: 50%;
		left: 50%;
		transform: translate(-50%, -50%);
		font-weight: bold;
		pointer-events: none;
		display: flex;
		justify-content: center;
		align-items: center;
		width: 100%;
		height: 100%;
	}
	.card-icon {
		width: 50%;
		height: 50%;
		object-fit: contain;
	}
	.text-value {
		font-size: 2em;
	}
	.text-value.small-text {
		font-size: 1em;
	}

	.player_hand {
		position: relative;
	}
	.player_hand .card {
		position: absolute;
	}

	#player .player_hand .card {
		cursor: grab;
		touch-action: none;
	}
	#player .player_hand .card:hover {
		transform-origin: left bottom;
		transform: rotate(-10deg) translateY(-0.7em);
		box-shadow: var(--lowShadowHover);
		z-index: 50;
	}
	#player .player_hand .card:hover ~ .card {
		transform: translateX(2em);
	}
	#player .player_hand .card:active {
		cursor: grabbing;
	}

	.card--hidden {
		opacity: 0 !important;
		pointer-events: none;
	}
	.card--dragging {
		opacity: 0.4;
		filter: brightness(0.75);
		animation: none !important;
	}
	.card--drag-target {
		outline: 2px dashed rgba(255, 255, 255, 0.85);
		outline-offset: 3px;
	}

	.card--playable {
		animation: playable-float 1.9s ease-in-out infinite;
		filter: brightness(1.15) drop-shadow(0 0 5px rgba(255, 255, 255, 0.65));
	}
	.card--playable:hover {
		filter: brightness(1.28) drop-shadow(0 0 10px rgba(255, 255, 255, 0.9));
	}
	#player .player_hand .card--playable:hover {
		animation: none;
	}

	@keyframes playable-float {
		0%,
		100% {
			transform-origin: left bottom;
			transform: translateY(0);
		}
		50% {
			transform-origin: left bottom;
			transform: translateY(-0.4em);
		}
	}

	.playable-glow {
		position: absolute;
		inset: -3px;
		border-radius: 0.85em;
		border: 2px solid rgba(255, 255, 255, 0.82);
		pointer-events: none;
		animation: glow-pulse 1.9s ease-in-out infinite;
		z-index: 20;
	}
	@keyframes glow-pulse {
		0%,
		100% {
			opacity: 0.55;
			box-shadow: 0 0 5px 2px rgba(255, 255, 255, 0.35);
		}
		50% {
			opacity: 1;
			box-shadow: 0 0 14px 5px rgba(255, 255, 255, 0.7);
		}
	}

	#draw_pile {
		position: absolute;
		left: 5em;
		top: 7em;
	}
	#draw_pile .card.top-card,
	#draw_pile .card.pile {
		position: absolute;
	}
	#draw_pile .card.pile {
		box-shadow:
			0px 2px white,
			0px 4px var(--shadowColor),
			0px 6px white,
			0px 8px var(--shadowColor),
			0px 10px white,
			0px 12px var(--shadowColor),
			0px 14px white,
			0px 16px var(--shadowColor),
			0px 18px white,
			0px 20px var(--shadowColor);
	}
	#draw_pile .card.pile:hover {
		transform: none;
	}
	#draw_pile .card.top-card {
		z-index: 100;
		box-shadow: none;
		transition:
			transform 200ms ease,
			box-shadow 200ms ease;
	}
	#draw_pile .card.top-card:hover {
		box-shadow: 0px 4px var(--shadowColor);
		cursor: pointer;
		transform: translateY(1em);
	}

	#discard_pile {
		position: absolute;
		left: 12em;
		top: 7.4em;
	}
	#discard_pile .card.top-card,
	#discard_pile .card.pile {
		position: absolute;
	}
	#discard_pile .card.pile {
		box-shadow:
			0px 2px white,
			0px 4px var(--shadowColor),
			0px 6px white,
			0px 8px var(--shadowColor);
	}
	#discard_pile .card.pile:hover {
		transform: none;
	}
	#discard_pile .card.top-card {
		z-index: 100;
		box-shadow: none;
		animation: discard-land 0.3s cubic-bezier(0.22, 1, 0.36, 1) both;
	}
	@keyframes discard-land {
		from {
			transform: scale(1.15) rotate(4deg);
			opacity: 0.6;
		}
		to {
			transform: scale(1) rotate(0deg);
			opacity: 1;
		}
	}
</style>
