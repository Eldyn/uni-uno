<script lang="ts">
	import { storeLobby } from "../../stores/lobby.svelte";

	let roomCodeInput = $state("");
	let roomCodeError = $state("");
	let isLoading = $state(false);

	function validateForm(): boolean {
		roomCodeError = "";
		const code = roomCodeInput.trim();
		if (!code) {
			roomCodeError = "Invite code is required.";
			return false;
		}
		if (code.length !== 6 || !/^[a-zA-Z0-9]+$/.test(code)) {
			roomCodeError = "Must be 6 characters.";
			return false;
		}
		return true;
	}

	async function handleSubmit(event: SubmitEvent) {
		event.preventDefault();
		if (!validateForm()) return;
		storeLobby.join(roomCodeInput);
	}
</script>

<form onsubmit={handleSubmit} class="join-form">
	<div class="input-group">
		<label for="invite-code">Invite Code:</label>
		<input
			id="invite-code"
			type="text"
			bind:value={roomCodeInput}
			placeholder="ABC123"
			disabled={isLoading}
			class:error={roomCodeError}
			maxlength="6"
		/>
		{#if roomCodeError}
			<span class="error-text">{roomCodeError}</span>
		{/if}
	</div>

	<button type="submit" disabled={isLoading} class="pixel-corners submit-btn">
		{isLoading ? "Joining..." : "JOIN LOBBY"}
	</button>
</form>

<style>
	.join-form {
		display: flex;
		flex-direction: column;
		gap: 25px;
	}

	.input-group {
		display: flex;
		flex-direction: column;
		gap: 8px;
	}

	/* LABEL COERENTE CON CREATE FORM */
	label {
		font-family: "Pixel", sans-serif;
		font-size: 14px;
		color: var(--text-h);
		text-transform: uppercase;
	}

	/* INPUT COERENTE CON CREATE FORM */
	input {
		padding: 12px;
		background: rgba(0, 0, 0, 0.3);
		border: 2px solid var(--border);
		color: var(--text-h);
		font-family: "Pixel", monospace;
		font-size: 16px;
		text-transform: uppercase; /* Forces visual uppercase */
		outline: none;
	}

	input:focus {
		border-color: var(--accent);
	}

	input.error {
		border-color: #ff4444;
	}

	.error-text {
		font-family: "Pixel", sans-serif;
		font-size: 11px;
		color: #ff4444;
		margin-top: 4px;
		text-transform: uppercase;
	}

	/* BOTTONE COERENTE */
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
