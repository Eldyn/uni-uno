<script lang="ts">
	import GameBoard from "./GameBoard.svelte";
	import { storeGame } from "../../stores/game.svelte";
	import { storeNavigation } from "../../stores/navigation.svelte.ts";
	import { ClientAction, ServerAction, ws } from "../../stores/ws.svelte";
</script>

<div class="game-screen">
	{#if storeGame.state.is_over && storeGame.state.winner}
		<div class="modal-overlay victory-overlay">
			<div class="modal-content victory-content">
				<h1>🏆 Partita Terminata!</h1>
				<h2>
					Ha vinto: <span class="winner-highlight">{storeGame.state.winner || "Sconosciuto"}</span>
				</h2>
				<p>Tutte le carte in mano sono state esaurite correttamente.</p>
				<button
					type="button"
					class="action-btn back-to-lobby-btn"
					onclick={storeGame.returnToLobby}
				>
					Back (ancora no destroy match instance + frontend data)
				</button>
			</div>
		</div>
	{/if}

	<div class="game-controls">
		{#if storeGame.state && !storeGame.actionRequired}
			<div
				style="color: white; font-weight: bold; font-size: 1.2rem; display: flex; align-items: center; gap: 15px;"
			>
				<span>Current Turn: {storeGame.state.current_turn}</span>

				<span
					style="background: {storeGame.turnTimeRemaining <= 5
						? '#dc251c'
						: '#333'}; padding: 4px 10px; border-radius: 6px; font-family: monospace;"
				>
					00:{storeGame.turnTimeRemaining.toString().padStart(2, "0")}
				</span>
			</div>
		{/if}

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

	{#if storeGame.actionRequired && storeGame.actionContext}
		<div class="modal-overlay">
			<div class="modal-content">
				<h2>{storeGame.actionContext.message}</h2>

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

	{#if storeGame.localPlayer && storeGame.localPlayer.hand?.length <= 2}
		{#if !storeGame.localPlayer.has_called_uno}
			<button
				type="button"
				class="uno-floating-button animate-pulse"
				onclick={() => {
					ws.emit(ClientAction.GameCallUno);
					console.log(ClientAction.GameCallUno);
				}}
			>
				🔥 UNO!
			</button>
		{:else}
			<div class="uno-floating-badge">✅ UNO Dichiarato</div>
		{/if}
	{/if}
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

	.victory-content {
		text-align: center;
		background: #1c1c1e;
		padding: 40px;
		border-radius: 16px;
		border: 3px solid #ffcc00;
		box-shadow: 0 0 40px rgba(255, 204, 0, 0.5);
		color: white;
		max-width: 450px;
		width: 90%;
	}

	.victory-content h1 {
		font-size: 2.5rem;
		margin-bottom: 15px;
		color: #ffcc00;
		font-weight: 900;
	}

	.winner-highlight {
		color: #34c759;
		text-shadow: 0 0 12px rgba(52, 199, 89, 0.6);
		font-weight: bold;
	}

	.back-to-lobby-btn {
		margin-top: 25px;
		padding: 14px 28px;
		background: #ffcc00;
		color: #000;
		border: none;
		border-radius: 8px;
		font-weight: bold;
		font-size: 1.1rem;
		cursor: pointer;
		transition:
			background 0.2s,
			transform 0.1s;
		box-shadow: 0 4px 15px rgba(255, 204, 0, 0.3);
	}

	.back-to-lobby-btn:hover {
		background: #e6b800;
		transform: scale(1.03);
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
		height: 100vh; /* Forzato a altezza schermo per il posizionamento del tavolo */
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
