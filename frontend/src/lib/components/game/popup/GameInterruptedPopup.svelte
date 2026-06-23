<script lang="ts">
	import { storeGame } from "../../../stores/game.svelte";
</script>

<div class="modal-overlay">
	<div class="modal-content interrupted-content">
		<h1>Interrupted</h1>
		<h2>Something went wrong.</h2>
		<p>A player disconnected or the match was forcefully cancelled.</p>
		<button type="button" class="btn pixel-corners" onclick={() => storeGame.returnToLobby()}>
			Back to Lobby
		</button>
	</div>
</div>

<style>
	.interrupted-content {
		text-align: center;
		color: var(--text-h);
		max-width: 380px;
		width: 90%;
		box-sizing: border-box;
		animation: shakeIn 0.4s cubic-bezier(0.36, 0.07, 0.19, 0.97) both;
		display: flex;
		flex-direction: column;
		align-items: center;
	}

	@keyframes shakeIn {
		0% {
			transform: scale(0.9);
			opacity: 0;
		}
		30% {
			transform: scale(1.02) rotate(-2deg);
			opacity: 1;
		}
		60% {
			transform: scale(1) rotate(2deg);
		}
		100% {
			transform: scale(1) rotate(0deg);
		}
	}

	.interrupted-content h1 {
		font-family: "FatPixel", sans-serif;
		font-size: 1rem;
		margin: 0 0 10px 0;
		color: var(--accent);
		text-shadow: 2px 2px 0px var(--pixel-shadow);
	}

	.interrupted-content h2 {
		font-family: "Pixel", sans-serif;
		font-size: 1.1rem;
		margin-bottom: 10px;
	}

	.interrupted-content p {
		font-family: var(--sans);
		color: var(--text);
		margin-bottom: 25px;
		font-size: 0.95rem;
	}
</style>
