<script lang="ts">
	import { dndzone, type DndEvent, SHADOW_ITEM_MARKER_PROPERTY_NAME } from "svelte-dnd-action";
	import { storeGame, type Card } from "../../stores/game.svelte";
	import GameCard from "./GameCard.svelte";
	import { useCardBus } from "./card-bus.svelte";

	let { playableCardIds = new Set<number>() }: { playableCardIds?: Set<number> } = $props();

	const bus = useCardBus();

	let handEl = $state<HTMLElement | null>(null);

	$effect(() => {
		if (handEl) bus.register("hand-local", handEl);
		return () => bus.unregister("hand-local");
	});

	let hand = $derived(storeGame.localPlayer?.hand ?? []);
	let items = $state<Card[]>([]);
	let dragging = $state(false);

	$effect(() => {
		const current = hand;
		if (dragging) return;

		const currentIds = new Set(current.map((c) => c.id));
		const itemIds = new Set(items.map((c) => c.id));
		const incoming = current.filter((c) => !itemIds.has(c.id));
		const surviving = items.filter((c) => currentIds.has(c.id));

		if (incoming.length > 0 || surviving.length !== items.length) {
			items = [...surviving, ...incoming];
		}
	});

	function onConsider(e: CustomEvent<DndEvent<Card>>) {
		dragging = true;
		items = e.detail.items;
	}

	function onFinalize(e: CustomEvent<DndEvent<Card>>) {
		items = e.detail.items.filter((item) => !(SHADOW_ITEM_MARKER_PROPERTY_NAME in item));
		dragging = false;
	}

	function handleCardClick(cardId: number) {
		if (!dragging) storeGame.playCard(cardId);
	}
</script>

<!--
    NOTE: The hand uses flexbox so svelte-dnd-action can measure real layout boxes.
          With position:absolute every card stacks at the same origin — the library
          can't tell which slot the pointer is over, so drops fail and the shadow
          never appears.

    NOTE: With flexbox each card is a normal flow item. We recover the overlapping
          fan with a negative margin-left on every card (set in the style block
          below). The card width (--cardSize = 5em) minus the desired step (2.2em)
          gives us the overlap amount.
-->
<div
	bind:this={handEl}
	class="player_hand"
	use:dndzone={{ items, flipDurationMs: 200, dropTargetStyle: {} }}
	onconsider={onConsider}
	onfinalize={onFinalize}
>
	{#each items as card, i (card.id)}
		<div class="card-slot" style="z-index: {i};">
			<GameCard
				{card}
				isHidden={bus.hiddenCardIds.has(card.id)}
				isPlayable={playableCardIds.has(card.id)}
				onCardClick={handleCardClick}
				style="position: relative; left: 0;"
			/>
		</div>
	{/each}
</div>

<style>
	.player_hand {
		position: absolute;
		top: 20%;
		left: 50%;
		transform: translate(-50%, -30%);

		display: flex;
		flex-direction: row;
		align-items: flex-end;

		overflow: visible;
		height: calc(var(--cardSize) * 1.5357 + 1em);
	}

	.card-slot {
		position: relative;
		flex-shrink: 0;
		width: 2.2em;
		height: calc(var(--cardSize) * 1.5357);
	}

	.card-slot:hover {
		z-index: 50 !important;
	}

	.card-slot:hover ~ .card-slot {
		transform: translateX(1em);
		transition: transform 150ms ease;
	}

	.card-slot {
		transition: transform 150ms ease;
	}
</style>
