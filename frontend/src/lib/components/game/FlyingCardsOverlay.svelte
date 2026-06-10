<script lang="ts">
	import { untrack } from "svelte";
	import GameCard from "./GameCard.svelte";
	import { useCardBus } from "./card-bus.svelte";
	import { storeGame } from "../../stores/game.svelte";

	const bus = useCardBus();

	function getRect(el: HTMLElement | null): DOMRect {
		return (
			el?.getBoundingClientRect() ??
			new DOMRect(window.innerWidth / 2, window.innerHeight / 2, 80, 120)
		);
	}

	/** Launch a card clone; hides the real card in-hand if hideId is given. */
	function launch(params: {
		color: string;
		value: string;
		turned: boolean;
		from: Parameters<typeof bus.launch>[0]["from"];
		to: Parameters<typeof bus.launch>[0]["to"];
		hideId?: number;
		delayMs?: number;
	}) {
		const fire = () => {
			const { color, value, turned, from, to, hideId } = params;

			if (hideId !== undefined) {
				bus.hide(hideId);
				setTimeout(() => bus.show(hideId), 550);
			}

			bus.launch({ color, value, turned, from, to });
		};

		if (params.delayMs) {
			setTimeout(fire, params.delayMs);
		} else {
			fire();
		}
	}

	let prevHandIds = $state(new Set<number>());
	let prevTopCardId = $state<number | null>(null);
	let prevOpponentCounts = $state(new Map<string, number>());
	let lastKnownHand = $state<{ id: number; color: string; value: string }[]>([]);

	let hand = $derived(storeGame.localPlayer?.hand ?? []);

	// NOTE: EFFECT 1
	//       Draw Pile -> PlayerHand
	$effect(() => {
		const currentHand = hand;
		const currentIds = new Set(currentHand.map((c) => c.id));

		untrack(() => {
			const newIds: number[] = [];
			for (const id of currentIds) {
				if (!prevHandIds.has(id)) newIds.push(id);
			}

			for (const id of newIds) {
				const card = currentHand.find((c) => c.id === id);
				if (!card) continue;

				launch({
					color: card.color,
					value: card.value,
					turned: true,
					from: "draw-pile",
					to: "hand-local",
					hideId: id
				});
			}

			prevHandIds = currentIds;
		});
	});

	// NOTE: EFFECT 2
	//       Any Card -> Discard Pile
	$effect(() => {
		const top = storeGame.state?.top_card;
		const currentHand = storeGame.localPlayer?.hand ?? [];

		untrack(() => {
			if (!top) return;

			if (top.id !== prevTopCardId) {
				const currentIds = new Set(currentHand.map((c) => c.id));
				const playedCard = lastKnownHand.find((c) => !currentIds.has(c.id) && c.id === top.id);

				if (playedCard) {
					launch({
						color: playedCard.color,
						value: playedCard.value,
						turned: false,
						from: "hand-local",
						to: "discard-pile"
					});
				}

				prevTopCardId = top.id;
			}

			lastKnownHand = currentHand.map((c) => ({
				id: c.id,
				color: c.color,
				value: c.value
			}));
		});
	});

	// NOTE: EFFECT 3
	//       Draw Pile -> OpponentHand
	$effect(() => {
		const players = storeGame.state?.players ?? [];
		const localUser = storeGame.localPlayer?.username;

		untrack(() => {
			const opponents = players.filter((p) => p.username !== localUser);

			for (let idx = 0; idx < opponents.length; idx++) {
				const p = opponents[idx];
				const prev = prevOpponentCounts.get(p.username) ?? p.card_count;
				const delta = p.card_count - prev;

				if (delta > 0) {
					for (let i = 0; i < delta; i++) {
						launch({
							color: "black",
							value: "",
							turned: true,
							from: "draw-pile",
							to: `hand-opponent-${idx}`,
							delayMs: i * 80
						});
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

	function rects(flight: (typeof bus.flights)[0]) {
		return {
			src: getRect(bus.getEl(flight.from)),
			dst: getRect(bus.getEl(flight.to))
		};
	}
</script>

{#each bus.flights as flight (flight.key)}
	{@const { src, dst } = rects(flight)}
	<div
		class="flying-card"
		style="
            --src-x: {src.left + src.width / 2}px;
            --src-y: {src.top + src.height / 2}px;
            --dst-x: {dst.left + dst.width / 2}px;
            --dst-y: {dst.top + dst.height / 2}px;
            left: var(--src-x);
            top: var(--src-y);
        "
		onanimationend={() => bus.land(flight.key)}
	>
		<GameCard
			card={{ id: flight.key, color: flight.color, value: flight.value }}
			turned={flight.turned}
			style="position: static;"
		/>
	</div>
{/each}

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
</style>
