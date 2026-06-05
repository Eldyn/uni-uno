<script lang="ts">
	import { storeGame } from "../../../stores/game.svelte";
</script>

{#if storeGame.actionRequired && storeGame.actionContext}
	{#if storeGame.actionRequired === "play_drawn_card" || storeGame.actionRequired === "choose_player"}
		<div class="modal-overlay">
			<div class="modal-content">
				<h2>{storeGame.actionContext.message}</h2>

				{#if storeGame.actionRequired === "play_drawn_card"}
					{@const drawnCard = storeGame.localPlayer?.hand?.find(
						(c) => c.id === storeGame.actionContext.card_id
					)}

					{#if drawnCard}
						<div class="card {drawnCard.color}">
							<div class="bckg"></div>
							<div class="card-value">
								{#if drawnCard.value === "Skip"}
									<img src="/images/skip.png" alt="Skip" class="card-icon" />
								{:else if drawnCard.value === "Rev" || drawnCard.value === "Reverse"}
									<img src="/images/reverse.png" alt="Reverse" class="card-icon" />
								{:else if drawnCard.value === "Wild_Draw4" || drawnCard.value === "+4"}
									<div class="wild-draw4-container">
										<img src="/images/wild.png" alt="Wild" class="card-icon wild-bg" />
										<span class="draw4-text">+4</span>
									</div>
								{:else if drawnCard.value === "Wild"}
									<img src="/images/wild.png" alt="Wild" class="card-icon" />
								{:else}
									<span class="text-value" class:small-text={drawnCard.value.length > 1}>
										{drawnCard.value}
									</span>
								{/if}
							</div>
						</div>

						<div class="action-buttons-group">
							<button class="action-btn play-btn" onclick={() => storeGame.submitInput("PLAY")}>
								Play It
							</button>
							<button class="action-btn keep-btn" onclick={() => storeGame.submitInput("KEEP")}>
								Keep It
							</button>
						</div>
					{/if}
				{/if}

				{#if storeGame.actionRequired === "choose_player"}
					<div class="player-options-list">
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
{/if}

<style>
	.modal-overlay {
		position: fixed;
		top: 0;
		left: 0;
		width: 100%;
		height: 100%;
		background: rgba(0, 0, 0, 0.6);
		display: flex;
		justify-content: center;
		align-items: center;
		z-index: 200;
	}

	.modal-content {
		background: #2a2a2a;
		padding: 24px;
		border-radius: 12px;
		text-align: center;
		box-shadow: 0 4px 20px rgba(0, 0, 0, 0.4);
		color: white;
	}

	.action-buttons-group {
		display: flex;
		gap: 10px;
		justify-content: center;
		margin-top: 20px;
	}

	.player-options-list {
		display: flex;
		flex-direction: column;
		gap: 10px;
		margin-top: 20px;
	}

	/* Rendering della carta nel modale */
	.card {
		display: inline-block;
		background-color: white;
		border: 1px solid #ccc;
		border-radius: 0.8em;
		padding: 0.3em;
		box-shadow: 0 1px 3px rgba(0, 0, 0, 0.12);
		position: relative;
		transform: scale(1.5);
		margin: 30px auto;
	}

	.card .bckg {
		width: 5em;
		height: calc(5em * 1.5357);
		border-radius: 0.5em;
		overflow: hidden;
		position: relative;
	}

	.card .bckg::before {
		content: "";
		width: 5em;
		height: calc(5em * 1.3);
		background-color: white;
		position: absolute;
		left: 50%;
		top: 50%;
		transform: translate(-50%, -50%) rotate(10deg);
		transform-origin: center center;
		border-radius: 90% 40%;
	}

	.card-value {
		position: absolute;
		top: 50%;
		left: 50%;
		transform: translate(-50%, -50%);
		display: flex;
		justify-content: center;
		align-items: center;
		pointer-events: none;
		width: 100%;
		height: 100%;
	}

	/* Gestione Icone delle carte Speciali */
	.card-icon {
		width: 2.5em;
		height: 2.5em;
		object-fit: contain;
	}

	.wild-draw4-container {
		position: relative;
		display: flex;
		justify-content: center;
		align-items: center;
		width: 100%;
		height: 100%;
	}

	.wild-bg {
		opacity: 0.85;
	}

	.draw4-text {
		position: absolute;
		font-size: 1.6rem;
		font-weight: 900;
		color: white;
		text-shadow: 
			-2px -2px 0 #000,  
			 2px -2px 0 #000,
			-2px  2px 0 #000,
			 2px  2px 0 #000,
			 0px 4px 8px rgba(0,0,0,0.8);
		letter-spacing: -1px;
	}

	.text-value {
		font-weight: bold;
		font-size: 2em;
	}

	.text-value.small-text {
		font-size: 1em;
	}

	.card.red { color: #dc251c; }
	.card.red .bckg { background-color: #dc251c; }
	.card.yellow { color: #fcf604; }
	.card.yellow .bckg { background-color: #fcf604; }
	.card.blue { color: #0493de; }
	.card.blue .bckg { background-color: #0493de; }
	.card.green { color: #018d41; }
	.card.green .bckg { background-color: #018d41; }
	.card.black { color: #1f1b18; }
	.card.black .bckg { background-color: #1f1b18; }
</style>