<script lang="ts">
	import GameBoard from "./GameBoard.svelte";
	import GameUnoButton from "./GameUnoButton.svelte";
	import GameOverPopup from "./GameOverPopup.svelte"; 
	import GameHud from "./GameHud.svelte";
	import GameActions from "./GameActions.svelte";
	import { storeGame } from "../../stores/game.svelte";
	import { storeNavigation } from "../../stores/navigation.svelte.ts";
	import { ClientAction, ServerAction, ws } from "../../stores/ws.svelte";
</script>

<div class="game-screen">
	<GameOverPopup />

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
</style>