<script lang="ts">
	import { storeGame } from "../../stores/game.svelte";

	let { gameColor = "green" }: { gameColor?: string } = $props();

	function handleDrawClick() {
		if (storeGame.state?.current_turn === storeGame.localPlayer?.username) {
			storeGame.drawCard();
		}
	}

	function handleCardClick(cardId: number) {
		storeGame.playCard(cardId);
	}

	let opponents = $derived(
		storeGame.state?.players.filter((p) => p.username !== storeGame.localPlayer?.username) ?? []
	);

	let leftPlayer = $derived(opponents[0]);
	let topPlayer = $derived(opponents[1]);
	let rightPlayer = $derived(opponents[2]);
</script>

<div class="game-field perspective {gameColor}">
	<div id="player">
		(You)
		<div class="player_hand">
			{#each storeGame.localPlayer?.hand ?? [] as card, i (card.id)}
				<div
					class="card {card.color}"
					role="button"
					tabindex="0"
					onclick={() => handleCardClick(card.id)}
					onkeydown={(e) => e.key === "Enter" && handleCardClick(card.id)}
				>
					<div class="bckg"></div>
					<div class="card-value">
						{#if card.value === "Skip"}
							<img src="/images/skip.png" alt="Skip" class="card-icon" />
						{:else if card.value === "Rev"}
							<img src="/images/reverse.png" alt="Reverse" class="card-icon" />
						{:else if card.value === "Wild"}
							<img src="/images/wild.png" alt="Wild" class="card-icon" />
						{:else}
							<span class="text-value" class:small-text={card.value.length > 1}>{card.value}</span>
						{/if}
					</div>
				</div>
			{/each}
		</div>
	</div>

	<div id="player_left">
		{leftPlayer ? leftPlayer.username : "Waiting..."}
		<div class="player_hand">
			{#each Array(leftPlayer?.card_count ?? 0) as _, n}
				<div class="card turned" data-index={n}>
					<div class="bckg"></div>
				</div>
			{/each}
		</div>
	</div>

	<div id="player_top">
		{topPlayer ? topPlayer.username : "Waiting..."}
		<div class="player_hand">
			{#each Array(topPlayer?.card_count ?? 0) as _, n}
				<div class="card turned" data-index={n}>
					<div class="bckg"></div>
				</div>
			{/each}
		</div>
	</div>

	<div id="player_right">
		{rightPlayer ? rightPlayer.username : "Waiting..."}
		<div class="player_hand">
			{#each Array(rightPlayer?.card_count ?? 0) as _, n}
				<div class="card turned" data-index={n}>
					<div class="bckg"></div>
				</div>
			{/each}
		</div>
	</div>

	<div id="piles_area">
		<div id="draw_pile" onclick={handleDrawClick} role="button" tabindex="0">
			<div class="card turned top-card">
				<div class="bckg"></div>
			</div>
			<div class="card turned pile"><div class="bckg"></div></div>
		</div>

		<div id="discard_pile">
			{#if storeGame.state?.top_card}
				<div class="card top-card {storeGame.state.top_card.color}">
					<div class="bckg"></div>
					<div class="card-value" style="z-index: 10;">
						{#if storeGame.state.top_card.value === "Skip"}
							<img src="/images/skip.png" alt="Skip" class="card-icon" />
						{:else if storeGame.state.top_card.value === "Rev"}
							<img src="/images/reverse.png" alt="Reverse" class="card-icon" />
						{:else if storeGame.state.top_card.value === "Wild"}
							<img src="/images/wild.png" alt="Wild" class="card-icon" />
						{:else}
							<span class="text-value" class:small-text={storeGame.state.top_card.value.length > 1}>
								{storeGame.state.top_card.value}
							</span>
						{/if}
					</div>
				</div>
			{:else}
				<div class="card top-card {gameColor}">
					<div class="bckg"></div>
				</div>
			{/if}
			<div class="card pile"><div class="bckg"></div></div>
		</div>
	</div>
</div>

<style>
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

	.game-field {
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

	.game-field #piles_area {
		position: relative;
		border-radius: 4em;
		transition: 200ms;
	}

	.game-field.yellow #piles_area {
		background-color: rgba(252, 246, 4, 0.4);
	}
	.game-field.blue #piles_area {
		background-color: rgba(4, 147, 222, 0.4);
	}
	.game-field.red #piles_area {
		background-color: rgba(220, 37, 28, 0.4);
	}
	.game-field.green #piles_area {
		background-color: rgba(1, 141, 65, 0.4);
	}

	#piles_area {
		grid-area: 2 / 2;
	}
	#player {
		grid-area: 3 / 2;
	}
	#player_left {
		grid-area: 2 / 1;
	}
	#player_top {
		grid-area: 1 / 2;
	}
	#player_right {
		grid-area: 2 / 3;
	}

	#draw_pile {
		position: absolute;
		left: 5em;
		top: 5em;
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
	}
	#draw_pile .card.top-card:hover {
		box-shadow: 0px 4px var(--shadowColor);
		cursor: pointer;
		transform: translateY(1em);
	}

	#discard_pile {
		position: absolute;
		left: 12em;
		top: 5.7em;
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
	}

	.player_hand {
		position: relative;
	}
	.player_hand .card {
		position: absolute;
	}

	.player_hand .card:nth-child(1) {
		left: calc(1 * 2.2em);
	}
	.player_hand .card:nth-child(2) {
		left: calc(2 * 2.2em);
	}
	.player_hand .card:nth-child(3) {
		left: calc(3 * 2.2em);
	}
	.player_hand .card:nth-child(4) {
		left: calc(4 * 2.2em);
	}
	.player_hand .card:nth-child(5) {
		left: calc(5 * 2.2em);
	}
	.player_hand .card:nth-child(6) {
		left: calc(6 * 2.2em);
	}
	.player_hand .card:nth-child(7) {
		left: calc(7 * 2.2em);
	}
	.player_hand .card:nth-child(8) {
		left: calc(8 * 2.2em);
	}
	.player_hand .card:nth-child(9) {
		left: calc(9 * 2.2em);
	}
	.player_hand .card:nth-child(10) {
		left: calc(10 * 2.2em);
	}
	.player_hand .card:nth-child(11) {
		left: calc(11 * 2.2em);
	}
	.player_hand .card:nth-child(12) {
		left: calc(12 * 2.2em);
	}
	.player_hand .card:nth-child(13) {
		left: calc(13 * 2.2em);
	}
	.player_hand .card:nth-child(14) {
		left: calc(14 * 2.2em);
	}
	.player_hand .card:nth-child(15) {
		left: calc(15 * 2.2em);
	}

	#player .player_hand .card {
		cursor: pointer;
	}
	#player .player_hand .card:hover {
		transform-origin: left bottom;
		transform: rotate(-10deg) translateY(-0.5em);
	}
	#player .player_hand .card:hover ~ .card {
		transform: translateX(2em);
	}

	#player_left .player_hand {
		transform-origin: left bottom;
		transform: rotate(90deg) translateY(-10em);
	}

	#player_top .player_hand {
		transform: translateY(1em);
	}

	#player_right .player_hand {
		transform-origin: left bottom;
		transform: rotate(-90deg) translate(-24em, 1em);
	}

	.game-screen {
		display: flex;
		flex-direction: column;
		width: 100%;
		height: 100vh;
		background: var(--bg);
		position: relative;
	}

	.game-controls {
		display: flex;
		justify-content: space-between;
		align-items: center;
		padding: 16px;
		border-bottom: 1px solid var(--border);
		background: var(--bg);
		position: relative;
		z-index: 100;
	}

	.uno-floating-button {
		position: fixed;
		bottom: 24px;
		right: 24px;
		z-index: 150;
		padding: 16px 28px;
		background: linear-gradient(135deg, #ff416c, #ff4b2b);
		color: white;
		border: none;
		border-radius: 50px;
		font-size: 18px;
		font-weight: 900;
		letter-spacing: 1px;
		cursor: pointer;
		box-shadow: 0 8px 25px rgba(255, 75, 43, 0.5);
		transition:
			transform 0.1s,
			box-shadow 0.2s;
	}

	.uno-floating-button:hover {
		transform: scale(1.08);
		box-shadow: 0 12px 30px rgba(255, 75, 43, 0.7);
	}

	.uno-floating-button:active {
		transform: scale(0.95);
	}

	.uno-floating-badge {
		position: fixed;
		bottom: 24px;
		right: 24px;
		z-index: 150;
		padding: 10px 20px;
		background: #018d41;
		color: white;
		border-radius: 30px;
		font-size: 14px;
		font-weight: bold;
		text-transform: uppercase;
		box-shadow: 0 4px 12px rgba(1, 141, 65, 0.4);
	}

	/* DESIGN OVERLAY DI VITTORIA (STILE MODALE BLOCCANTE) */
	.victory-overlay {
		background: rgba(0, 0, 0, 0.92);
		z-index: 300;
		display: flex;
		justify-content: center;
		align-items: center;
		position: fixed;
		top: 0;
		left: 0;
		width: 100%;
		height: 100%;
	}

	@keyframes pulse {
		0% {
			transform: scale(1);
		}
		50% {
			transform: scale(1.04);
		}
		100% {
			transform: scale(1);
		}
	}

	.animate-pulse {
		animation: pulse 1.2s infinite ease-in-out;
	}

	.color-buttons {
		display: flex;
		gap: 8px;
	}

	.color-button {
		width: 36px;
		height: 36px;
		border: 2px solid transparent;
		border-radius: 6px;
		font-weight: 600;
		cursor: pointer;
		transition: all 0.2s;
		font-size: 12px;
	}

	.color-red {
		background: #dc251c;
		color: white;
	}
	.color-yellow {
		background: #fcf604;
		color: black;
	}
	.color-green {
		background: #018d41;
		color: white;
	}
	.color-blue {
		background: #0493de;
		color: white;
	}

	/* Game styles (preserved for game screen) */
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

	.card {
		display: inline-block;
		background-color: white;
		border: 1px solid #ccc;
		border-radius: 0.8em;
		padding: 0.3em;
		box-shadow: var(--lowShadow);
		transition: 200ms;
		position: relative;
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

	.card .center-icon {
		position: absolute;
		top: 50%;
		left: 50%;
		transform: translate(-50%, -50%);
		font-size: 3em;
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

	.card.turned:hover {
		cursor: default;
	}
	.card.turned .bckg {
		background-color: var(--blackCard);
	}
	.card.turned .bckg::before {
		background-color: var(--redCard);
	}

	.game-field {
		height: 100vh; /* Forzato a altezza schermo per il posizionamento del tavolo */
		display: grid;
		justify-content: center;
		align-content: center;
		grid-gap: 0.5em;
		grid-template-columns: var(--playerSpace) var(--fieldSize) var(--playerSpace);
		grid-template-rows: var(--playerSpace) var(--fieldSize) var(--playerSpace);
	}

	.game-field.perspective {
		transform: rotateX(20deg);
	}

	.game-field #piles_area {
		position: relative;
		border-radius: 4em;
		transition: 200ms;
	}

	.game-field.yellow #piles_area {
		background-color: rgba(252, 246, 4, 0.4);
	}
	.game-field.blue #piles_area {
		background-color: rgba(4, 147, 222, 0.4);
	}
	.game-field.red #piles_area {
		background-color: rgba(220, 37, 28, 0.4);
	}
	.game-field.green #piles_area {
		background-color: rgba(1, 141, 65, 0.4);
	}

	#piles_area {
		grid-area: 2 / 2;
	}
	#player {
		grid-area: 3 / 2;
	}
	#player_left {
		grid-area: 2 / 1;
	}
	#player_top {
		grid-area: 1 / 2;
	}
	#player_right {
		grid-area: 2 / 3;
	}

	/* DRAW PILE */
	#draw_pile {
		position: absolute;
		left: 5em;
		top: 5em;
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
	}
	#draw_pile .card.top-card:hover {
		box-shadow: 0px 4px var(--shadowColor);
		cursor: pointer;
		transform: translateY(1em);
	}

	#discard_pile {
		position: absolute;
		left: 12em;
		top: 5.7em;
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
	}

	.player_hand {
		position: relative;
	}
	.player_hand .card {
		position: absolute;
	}

	.player_hand .card:nth-child(1) {
		left: calc(1 * 2.2em);
	}
	.player_hand .card:nth-child(2) {
		left: calc(2 * 2.2em);
	}
	.player_hand .card:nth-child(3) {
		left: calc(3 * 2.2em);
	}
	.player_hand .card:nth-child(4) {
		left: calc(4 * 2.2em);
	}
	.player_hand .card:nth-child(5) {
		left: calc(5 * 2.2em);
	}
	.player_hand .card:nth-child(6) {
		left: calc(6 * 2.2em);
	}
	.player_hand .card:nth-child(7) {
		left: calc(7 * 2.2em);
	}
	.player_hand .card:nth-child(8) {
		left: calc(8 * 2.2em);
	}
	.player_hand .card:nth-child(9) {
		left: calc(9 * 2.2em);
	}
	.player_hand .card:nth-child(10) {
		left: calc(10 * 2.2em);
	}
	.player_hand .card:nth-child(11) {
		left: calc(11 * 2.2em);
	}
	.player_hand .card:nth-child(12) {
		left: calc(12 * 2.2em);
	}
	.player_hand .card:nth-child(13) {
		left: calc(13 * 2.2em);
	}
	.player_hand .card:nth-child(14) {
		left: calc(14 * 2.2em);
	}
	.player_hand .card:nth-child(15) {
		left: calc(15 * 2.2em);
	}

	#player .player_hand .card {
		cursor: pointer;
	}
	#player .player_hand .card:hover {
		transform-origin: left bottom;
		transform: rotate(-10deg) translateY(-0.5em);
	}
	#player .player_hand .card:hover ~ .card {
		transform: translateX(2em);
	}

	#player_left .player_hand {
		transform-origin: left bottom;
		transform: rotate(90deg) translateY(-10em);
	}

	#player_top .player_hand {
		transform: translateY(1em);
	}

	#player_right .player_hand {
		transform-origin: left bottom;
		transform: rotate(-90deg) translate(-24em, 1em);
	}
</style>
