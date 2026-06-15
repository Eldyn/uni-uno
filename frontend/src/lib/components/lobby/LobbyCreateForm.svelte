<script lang="ts">
	import { storeLobby } from "../../stores/lobby.svelte";

	let name = $state("");
	let isPrivate = $state(false);

	async function handleSubmit(e: Event) {
		e.preventDefault();
		// We send isPrivate (true/false) to the store
		await storeLobby.create({ name, is_public: !isPrivate });
		// Reset campi
		name = "";
		isPrivate = false;
	}
</script>

<form onsubmit={handleSubmit} class="create-form">
	<!-- NOME LOBBY -->
	<div class="input-group">
		<label for="name">Lobby Name</label>
		<input
			type="text"
			id="name"
			bind:value={name}
			placeholder="Enter name..."
			required
			autocomplete="off"
		/>
	</div>

	<!-- CHECKBOX AFFIANCATO -->
	<div class="checkbox-row">
		<label for="private">Private Lobby</label>
		<input type="checkbox" id="private" bind:checked={isPrivate} />
	</div>

	<button type="submit" class="pixel-corners submit-btn" disabled={!name}> CREATE </button>
</form>

<style>
	.pixel-corners {
		/* Removes native smooth rounding */
		border-radius: 0 !important;
		/* Cuts the corners in 4-pixel blocky steps */
		clip-path: polygon(
			0 4px,
			4px 4px,
			4px 0,
			calc(100% - 4px) 0,
			calc(100% - 4px) 4px,
			100% 4px,
			100% calc(100% - 4px),
			calc(100% - 4px) calc(100% - 4px),
			calc(100% - 4px) 100%,
			4px 100%,
			4px calc(100% - 4px),
			0 calc(100% - 4px)
		);
	}
	.create-form {
		display: flex;
		flex-direction: column;
		gap: 15px; /* More space between the elements */
	}

	.input-group {
		display: flex;
		flex-direction: column;
		gap: 8px;
	}

	.input-group label {
		font-family: "Pixel", sans-serif;
		font-size: 14px;
		color: var(--text-h);
		text-transform: uppercase;
	}

	input[type="text"] {
		padding: 12px;
		background: rgba(0, 0, 0, 0.3);
		border: 2px solid var(--border);
		color: var(--text-h);
		font-family: "Pixel", monospace;
		outline: none;
	}

	input[type="text"]:focus {
		border-color: var(--accent);
	}

	/* RIGA CHECKBOX AFFIANCATA */
	.checkbox-row {
		display: flex;
		align-items: center;
		gap: 12px;
		cursor: pointer;
	}

	.checkbox-row input[type="checkbox"] {
		width: 20px;
		height: 20px;
		margin: 0;
		cursor: pointer;
		accent-color: var(--accent);
	}

	.checkbox-row label {
		font-family: "Pixel", sans-serif;
		font-size: 14px;
		color: var(--text-h);
		cursor: pointer;
		user-select: none;
	}

	/* BOTTONE */
	.submit-btn {
		padding: 15px;
		background: var(--accent);
		color: white;
		border: none;
		font-family: "Pixel", sans-serif;
		font-size: 20px;
		cursor: pointer;
		transition: filter 0.2s;
	}

	.submit-btn:hover:not(:disabled) {
		filter: brightness(1.2);
	}

	.submit-btn:disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}
</style>

