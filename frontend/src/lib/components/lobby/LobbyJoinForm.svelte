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
			class="input-pixel input-dark uppercase"
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

	<button type="submit" disabled={isLoading} class="btn pixel-corners submit-btn">
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

	label {
		font-family: "Pixel", sans-serif;
		font-size: 14px;
		color: var(--text-h);
		text-transform: uppercase;
	}

	.error-text {
		font-family: "Pixel", sans-serif;
		font-size: 11px;
		color: var(--danger);
		margin-top: 4px;
		text-transform: uppercase;
	}

	/* Size override on top of the shared flat .btn */
	.submit-btn {
		padding: 15px;
		font-size: 20px;
	}
</style>
