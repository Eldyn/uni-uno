<script lang="ts">
	import { validateLobbyName } from "../../utils/validation";
	import { storeLobby } from "../../stores/lobby.svelte";
	import { storeToast } from "../../stores/toast.svelte";

	let lobbyNameInput = $state("");
	let lobbyNameError = $state("");
	let isLoading = $state(false);
	let isLobbyPublic = $state(false);

	function validateForm(): boolean {
		lobbyNameError = "";

		const lobbyValidation = validateLobbyName(lobbyNameInput);
		if (!lobbyValidation.valid) {
			lobbyNameError = lobbyValidation.error || "";
			return false;
		}

		return true;
	}

	async function handleSubmit(event: SubmitEvent) {
		event.preventDefault();

		if (!validateForm()) {
			return;
		}

		await storeLobby.create({ is_public: isLobbyPublic, name: lobbyNameInput.trim() });
		storeToast.success("Lobby created successfully!");
	}
</script>

<form onsubmit={handleSubmit} class="create-lobby-form">
	<h3>Create New Lobby</h3>

	<div class="form-group">
		<label for="lobby-name">Room Name:</label>
		<input
			id="lobby-name"
			type="text"
			bind:value={lobbyNameInput}
			placeholder="Enter Lobby name"
			disabled={isLoading}
			class:error={lobbyNameError}
		/>
		{#if lobbyNameError}
			<span class="error-text">{lobbyNameError}</span>
		{/if}
	</div>

	<div class="form-actions">
		<div class="checkbox-group">
			<label for="lobby-is-public">Is Public?</label>
			<input id="lobby-is-public" type="checkbox" bind:checked={isLobbyPublic} />
		</div>

		<button type="submit" disabled={isLoading} class="submit-button">
			{isLoading ? "Creating..." : "Create Lobby"}
		</button>
	</div>
</form>

<style>
	.create-lobby-form {
		display: flex;
		flex-direction: column;
		gap: 16px;
		width: 92%;
		max-width: 400px;
	}
	h3 {
		margin: 0;
		font-size: 16px;
		color: var(--text-h);
	}

	.form-group {
		display: flex;
		flex-direction: column;
		gap: 6px;
	}

	label {
		font-size: 14px;
		font-weight: 500;
		color: var(--text-h);
	}

	input {
		padding: 10px 12px;
		border: 2px solid var(--border);
		border-radius: 6px;
		font-size: 14px;
		color: var(--text-h);
		background: var(--bg);
		transition: border-color 0.2s;
		width: 100%;
	}

	input:focus {
		outline: none;
		border-color: var(--accent);
	}

	input:disabled {
		opacity: 0.6;
		cursor: not-allowed;
	}

	input.error {
		border-color: #dc2626;
	}

	.error-text {
		font-size: 12px;
		color: #dc2626;
	}

	.form-actions {
		display: flex;
		align-items: center;
		justify-content: space-between;
		gap: 12px;
	}

	.checkbox-group {
		display: flex;
		align-items: center;
		gap: 8px;
        padding: 10px 14px;
	}

	.checkbox-group label {
		cursor: pointer;
	}

	input[type="checkbox"] {
		padding: 0;
		width: 16px;
		height: 16px;
		cursor: pointer;
	}

	.submit-button {
		padding: 10px 14px;
		font-size: 13px;
		background: var(--accent);
		color: white;
		border: none;
		border-radius: 6px;
		font-weight: 500;
		cursor: pointer;
		transition: opacity 0.2s;
		white-space: nowrap;
	}

	.submit-button:hover:not(:disabled) {
		opacity: 0.9;
	}

	.submit-button:disabled {
		opacity: 0.6;
		cursor: not-allowed;
	}
</style>
