<script lang="ts">
	import { storeLobby, type Lobby } from "../../stores/lobby.svelte";

	let { lobby }: { lobby: Lobby } = $props();

	const isFull = $derived(lobby.member_count >= 4);
	const canJoin = $derived(!!lobby.invite_code);

	async function handleJoin() {
		storeLobby.join(lobby.invite_code);
	}
</script>

<div class="lobby-card card-outline pixel-corners" class:full={isFull}>
	<div class="lobby-header">
		<h3 class="lobby-name">{lobby.name}</h3>
	</div>

	<div class="lobby-info">
		<p class="info-item">
			<span class="info-label">Host:</span>
			<span class="info-value">{lobby.host}</span>
		</p>
		<p class="info-item">
			<span class="info-label">Players:</span>
			<span class="info-value">{lobby.member_count}/4</span>
		</p>
	</div>

	{#if canJoin}
		{#if isFull}
			<button type="button" class="btn pixel-corners" disabled>Full</button>
		{:else}
			<button type="button" class="btn pixel-corners" onclick={handleJoin}>Join</button>
		{/if}
	{/if}
</div>

<style>
	.lobby-card {
		display: flex;
		flex-direction: column;
		gap: 12px;
	}

	.lobby-card.full {
		opacity: 0.6;
	}

	.lobby-card.full:hover {
		border-color: var(--border);
		box-shadow: none;
	}

	.lobby-header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		gap: 12px;
	}

	.lobby-name {
		margin: 0;
		font-size: 16px;
		color: var(--text-h);
		word-break: break-word;
	}

	.lobby-info {
		display: flex;
		flex-direction: column;
		gap: 6px;
	}

	.info-item {
		margin: 0;
		display: flex;
		justify-content: space-between;
		font-size: 14px;
	}

	.info-label {
		color: var(--text);
		font-weight: 500;
	}

	.info-value {
		color: var(--text-h);
	}
</style>
