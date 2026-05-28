<script lang="ts">
	import { storeLobby } from "../../stores/lobby.svelte";
	import { storeAuth } from "../../stores/auth.svelte";
	import { ws } from "../../stores/ws.svelte";

	let isHost = $derived(storeAuth.username === storeLobby.current?.host);

	// Local UI state for handling the name-editing interaction
	let isEditingName = $state(false);
	let editedName = $state("");

	// Activated when the host clicks the name
	function startEditing() {
		if (!isHost) return;
		editedName = storeLobby.current?.name ?? "";
		isEditingName = true;
	}

	// Triggered when deselecting (blur) or pressing Enter
	function saveName() {
		isEditingName = false;
		const trimmed = editedName.trim();

		// Only send the update if it's a valid change to save network bandwidth
		if (trimmed && trimmed !== storeLobby.current?.name && trimmed.length <= 32) {
			storeLobby.updateSettings({ name: trimmed });
		}
	}

	// Triggered immediately when clicking the visibility checkbox
	function togglePublic(e: Event) {
		const target = e.target as HTMLInputElement;
		storeLobby.updateSettings({ is_public: target.checked });
	}
</script>

<link rel="preload" href="fonts/JetBrainsMono.woff2" as="font" type="font/woff2" />

<div>
	<div class="lobby-controls">
		<button class="join-button" onclick={storeLobby.leave}>Leave</button>

		<button
			class="join-button"
			onclick={() => {
				ws.emit("lobby_start_game");
			}}
			disabled={!isHost || storeLobby.current!.members.length < 2}
		>
			Start Game
		</button>
	</div>

	<div class="title-container">
		{#if isEditingName && isHost}
			<input
				class="name-input"
				bind:value={editedName}
				onblur={saveName}
				onkeydown={(e) => e.key === "Enter" && saveName()}
				maxlength="32"
				placeholder="Lobby Name"
				autofocus
			/>
		{:else}
			<h1
				class="lobby-title"
				class:editable={isHost}
				onclick={startEditing}
				title={isHost ? "Click to edit lobby name" : ""}
			>
				{storeLobby.current!.name}
			</h1>
		{/if}

		<span class="invite-badge">
			{storeLobby.current!.invite_code}
		</span>
	</div>

	<div class="settings-row">
		<label class="toggle-label">
			<input
				type="checkbox"
				checked={storeLobby.current!.is_public}
				disabled={!isHost}
				onchange={togglePublic}
			/>
			<span>Public Lobby</span>
		</label>
	</div>

	<ul class="memberlist">
		{#each storeLobby.current!.members as member}
			<li class="member">
				<span class="member-name">{member.username}</span>

				{#if member.is_host}
					<span style="color: lightgoldenrodyellow"> 󱟜 </span>
				{/if}

				{#if isHost && !member.is_host}
					<button class="transfer-button" onclick={() => storeLobby.promote(member.username)}>
						<span style="color: lightgoldenrodyellow">Promote</span>
					</button>
				{/if}

				{#if isHost && !member.is_host}
					<button class="transfer-button" onclick={() => storeLobby.kick(member.username)}>
						<span style="color: lightsalmon">Kick</span>
					</button>
				{/if}
				<span class:off={!member.is_connected} class:on={member.is_connected}>
					{member.is_connected ? "\uf14a" : "\uf2d3"}
				</span>
			</li>
		{/each}
	</ul>
</div>

<style>
	@font-face {
		font-family: "JetBrainsMono";
		src: url("/fonts/JetBrainsMono.woff2") format("woff2");
		font-weight: 400;
		font-style: normal;
		font-display: swap;
	}

	.lobby-controls {
		display: flex;
		gap: 10px;
		margin-bottom: 20px;
	}

	.title-container {
		display: flex;
		align-items: center;
		gap: 15px;
		margin-bottom: 15px;
	}

	.lobby-title {
		margin: 0;
		font-size: 2rem;
	}

	.lobby-title.editable {
		cursor: pointer;
		border-bottom: 2px dashed transparent;
		transition: border-color 0.2s;
	}

	.lobby-title.editable:hover {
		border-color: var(--accent, #555);
	}

	.name-input {
		font-family: inherit;
		font-size: 2rem;
		font-weight: bold;
		background: #222;
		color: white;
		border: 1px solid #555;
		border-radius: 6px;
		padding: 2px 8px;
		outline: none;
	}

	.invite-badge {
		font-family: "JetBrainsMono";
		background: #333;
		color: #aaa;
		padding: 4px 10px;
		border-radius: 4px;
		font-size: 1.2rem;
		font-weight: bold;
	}

	.settings-row {
		margin-bottom: 25px;
		background: #1e1e1e;
		padding: 10px 15px;
		border-radius: 6px;
		display: inline-block;
	}

	.toggle-label {
		display: flex;
		align-items: center;
		gap: 8px;
		color: #ccc;
		cursor: pointer;
		user-select: none;
	}

	.toggle-label input:disabled {
		cursor: not-allowed;
	}

	.toggle-label input:disabled + span {
		color: #666;
	}

	.join-button {
		padding: 8px 16px;
		background: var(--accent);
		color: white;
		border: none;
		border-radius: 6px;
		font-size: 14px;
		font-weight: 500;
		cursor: pointer;
		transition:
			background 0.2s,
			opacity 0.2s;
	}

	.join-button:disabled {
		background: #444444;
		color: #888888;
		cursor: not-allowed;
		opacity: 0.6;
	}

	.memberlist {
		list-style-type: none;
		padding: 0;
	}

	.member {
		font-family: "JetBrainsMono";
		color: lightgray;
		display: flex;
		align-items: center;
		gap: 10px;
		margin-bottom: 8px;
	}

	.member-name {
		min-width: 120px;
	}

	.transfer-button {
		padding: 4px 8px;
		background: #333333;
		color: #cccccc;
		border: 1px solid #555555;
		border-radius: 4px;
		font-size: 11px;
		cursor: pointer;
		font-family: "JetBrainsMono";
		transition:
			background 0.2s,
			color 0.2s;
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.transfer-button:hover {
		background: rgba(250, 250, 210, 0.39);
		color: #111111;
		border-color: rgba(250, 250, 210, 0.322);
	}

	.off {
		color: salmon;
	}

	.on {
		color: lightgreen;
	}
</style>
