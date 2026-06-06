<script lang="ts">
	import GameBoard from "./GameBoard.svelte";
	import GameUnoButton from "./GameUnoButton.svelte";
	import GameVictoryPopup from "./GameVictoryPopup.svelte";
	import GameHud from "./GameHud.svelte";
	import GameActions from "./GameActions.svelte";
	import { storeGame } from "../../stores/game.svelte";
	import { storeNavigation } from "../../stores/navigation.svelte.ts";
	import { ClientAction, ServerAction, ws } from "../../stores/ws.svelte";
</script>

<div class="game-screen">
	<GameVictoryPopup />

	<div class="game-controls">
		<GameHud />

		<GameActions />
	</div>

	<GameBoard gameColor={storeGame.state?.active_color ?? "green"} />

	<GameUnoButton />
</div>

<style>
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

	body {
		perspective: 100em;
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
		height: 100vh;
		display: grid;
		justify-content: center;
		align-content: center;
		grid-gap: 0.5em;
		grid-template-columns: var(--playerSpace) var(--fieldSize) var(--playerSpace);
		grid-template-rows: var(--playerSpace) var(--fieldSize) var(--playerSpace);
	}

	.game-field.perspective {
		transform: rotateX(30deg);
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

