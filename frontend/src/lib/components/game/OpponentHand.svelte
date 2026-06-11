<script lang="ts">
	import GameCard from "./GameCard.svelte";
	import { useCardBus, type ElementRole } from "./card-bus.svelte";
	import { storeGame, type GamePlayer } from "../../stores/game.svelte";

	let {
		player,
		index,
		handTransform = "translate(-50%, -50%)",
		labelPos = "top: 0; left: 50%; transform: translateX(-50%);",
		boxPos = "top: 0; left: 50%; transform: translateX(-50%);",
		isTop = false // <-- NEW
	}: {
		player: GamePlayer | null;
		index: number;
		handTransform?: string;
		labelPos?: string;
		boxPos?: string;
		isTop?: boolean; // <-- NEW
	} = $props();

	const bus = useCardBus();
	const role: ElementRole = `hand-opponent-${index}`;

	let handEl = $state<HTMLElement | null>(null);

	$effect(() => {
		if (handEl) {
			bus.register(role, handEl);
		}
		return () => bus.unregister(role);
	});

	let cardCount = $derived(player?.card_count ?? 0);
	let handWidth = $derived(`calc(${cardCount} * 2.2em + 7.2em)`);
</script>

<div class="opponent-slot">
	<div
		class="box"
		class:is-turn={player && storeGame.state?.current_turn === player.username}
		style={boxPos}
	>
		<div class="player-label" class:is-top={isTop}>
			{player ? player.username : "Waiting..."}
		</div>
	</div>

	<div
		bind:this={handEl}
		class="player_hand"
		style="
            width: {handWidth};
            height: calc(var(--cardSize) * 1.5357);
            transform: {handTransform};
        "
	>
		{#each Array(cardCount) as _, n}
			<GameCard card={{ id: -1, color: "wild", value: "0" }} index={n} turned={true} />
		{/each}
	</div>
</div>

<style>
	/* ... keep all existing styles exactly as they are ... */
	.opponent-slot {
		position: relative;
		width: 100%;
		height: 100%;
	}

	.box {
		position: absolute;
		width: 50px;
		height: 50px;
		background-color: #000;
		z-index: 100;
		transition:
			box-shadow 0.3s ease,
			background-color 0.3s ease;
	}

	.box.is-turn {
		background-color: #ffd700;
		box-shadow: 0 0 20px 6px rgba(255, 215, 0, 0.75);
	}

	.player-label {
		position: absolute;
		left: 50%;
		transform: translateX(-50%);
		white-space: nowrap;
		font-weight: bold;
		color: white;
		text-shadow: 0 1px 4px rgba(0, 0, 0, 0.8);
		font-size: 0.95em;
		letter-spacing: 0.04em;
		z-index: 110;
		bottom: calc(100% + 0.4em);
	}

	.player-label.is-top {
		bottom: auto;
		top: calc(100% + 0.4em);
	}

	.player_hand {
		position: absolute;
		top: 45%;
		left: 50%;
	}
</style>

