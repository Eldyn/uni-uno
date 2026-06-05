<script lang="ts">
	import { storeGame } from "../../stores/game.svelte";
</script>

{#if storeGame.state && !storeGame.actionRequired}
	<div class="hud-container">
		<span>Current Turn: {storeGame.state.current_turn}</span>

		<span
			class="timer"
			style="background: {storeGame.turnTimeRemaining <= 5 ? '#dc251c' : '#333'};"
		>
			00:{storeGame.turnTimeRemaining.toString().padStart(2, "0")}
		</span>
	</div>
{/if}

<style>
	.hud-container {
		color: white;
		font-weight: bold;
		font-size: 1.2rem;
		display: flex;
		align-items: center;
		gap: 15px;
	}

	.timer {
		padding: 4px 10px;
		border-radius: 6px;
		font-family: monospace;
		transition: background-color 0.3s ease;
	}
</style>