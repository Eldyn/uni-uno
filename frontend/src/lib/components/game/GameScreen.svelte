<script lang="ts">
	import GameBoard from "./GameBoard.svelte";
	import { storeGame } from "../../stores/game.svelte";

	let { onBack }: { onBack?: () => void } = $props();
</script>

<div class="game-screen">
	<div class="game-controls">
		<button type="button" class="back-button" onclick={onBack}>← Back to Lobbies</button>

		<!-- STANDARD TURN INFO & TIMER -->
		{#if !storeGame.actionRequired}
			<div
				style="color: white; font-weight: bold; font-size: 1.2rem; display: flex; align-items: center; gap: 15px;"
			>
				<span>Current Turn: {storeGame.state?.current_turn}</span>

				<!-- Turn Timer Badge -->
				<span
					style="background: {storeGame.turnTimeRemaining <= 5
						? '#dc251c'
						: '#333'}; padding: 4px 10px; border-radius: 6px; font-family: monospace;"
				>
					00:{storeGame.turnTimeRemaining.toString().padStart(2, "0")}
				</span>
			</div>
		{/if}

		<!-- 1. CHOOSE COLOR PROMPT -->
		{#if storeGame.actionRequired === "choose_color"}
			<div class="color-buttons">
				<span style="color: white; font-weight: bold; margin-right: 10px;">Choose Color:</span>
				{#each ["red", "yellow", "green", "blue"] as color}
					<button
						type="button"
						class="color-button color-{color}"
						onclick={() => storeGame.submitInput(color.toUpperCase())}
					>
						{color.charAt(0).toUpperCase()}
					</button>
				{/each}
			</div>
		{/if}
	</div>

	<!-- GENERIC ACTION MODALS -->
	{#if storeGame.actionRequired && storeGame.actionContext}
		<div class="modal-overlay">
			<div class="modal-content">
				<!-- Use the dynamic message provided directly by the C++ Effect! -->
				<h2>{storeGame.actionContext.message}</h2>

				<!-- 2. PLAY DRAWN CARD MODAL -->
				{#if storeGame.actionRequired === "play_drawn_card"}
					{@const drawnCard = storeGame.localPlayer?.hand?.find(
						(c) => c.id === storeGame.actionContext.card_id
					)}

					{#if drawnCard}
						<div class="card {drawnCard.color}" style="transform: scale(1.5); margin: 30px auto;">
							<div class="bckg"></div>
							<div
								class="card-value"
								style="position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); font-weight: bold; font-size: {drawnCard
									.value.length > 1
									? '1em'
									: '2em'}; pointer-events: none;"
							>
								{drawnCard.value}
							</div>
						</div>

						<div style="display: flex; gap: 10px; justify-content: center; margin-top: 20px;">
							<button class="action-btn play-btn" onclick={() => storeGame.submitInput("PLAY")}
								>Play It</button
							>
							<button class="action-btn keep-btn" onclick={() => storeGame.submitInput("KEEP")}
								>Keep It</button
							>
						</div>
					{/if}
				{/if}

				<!-- 3. FUTURE PROOFING: 0/7 SWAP MODAL -->
				{#if storeGame.actionRequired === "choose_player"}
					<div style="display: flex; flex-direction: column; gap: 10px; margin-top: 20px;">
						{#each storeGame.actionContext.options as opponentName}
							<button
								class="action-btn keep-btn"
								onclick={() => storeGame.submitInput(opponentName)}
							>
								{opponentName}
							</button>
						{/each}
					</div>
				{/if}
			</div>
		</div>
	{/if}

	<GameBoard gameColor={storeGame.state?.active_color ?? "green"} />
</div>

<style>
	.game-screen {
		display: flex;
		flex-direction: column;
		width: 100%;
		height: 100vh;
		background: var(--bg);
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

	.back-button {
		padding: 8px 16px;
		background: var(--accent);
		color: white;
		border: none;
		border-radius: 6px;
		font-size: 14px;
		font-weight: 500;
		cursor: pointer;
		transition: opacity 0.2s;
	}

	.back-button:hover {
		opacity: 0.9;
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

	.color-button.active {
		border-color: var(--text-h);
		box-shadow:
			0 0 0 2px var(--bg),
			0 0 0 4px var(--text-h);
	}

	.color-button:hover {
		transform: scale(1.1);
	}
</style>
