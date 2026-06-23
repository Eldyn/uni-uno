<script lang="ts">
	import { storeLobby } from "../../stores/lobby.svelte";

	let name = $state("");
	let isPrivate = $state(false);

	async function handleSubmit(e: Event) {
		e.preventDefault();
		await storeLobby.create({ name, is_public: !isPrivate });
		name = "";
		isPrivate = false;
	}
</script>

<form onsubmit={handleSubmit} class="create-form">
	<div class="input-group">
		<label for="name">Lobby Name</label>
		<input
			type="text"
			id="name"
			class="input-pixel input-dark"
			bind:value={name}
			placeholder="Enter name..."
			required
			autocomplete="off"
		/>
	</div>

	<div class="checkbox-row">
		<label for="private">Private Lobby</label>
		<input type="checkbox" id="private" bind:checked={isPrivate} />
	</div>

	<button type="submit" class="btn pixel-corners submit-btn" disabled={!name}> CREATE </button>
</form>

<style>
	.create-form {
		display: flex;
		flex-direction: column;
		gap: 15px;
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

	/* Size override on top of the shared flat .btn */
	.submit-btn {
		padding: 15px;
		font-size: 20px;
	}
</style>
