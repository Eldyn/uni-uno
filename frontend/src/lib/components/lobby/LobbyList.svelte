<script lang="ts">
	import LobbyCard from "./LobbyCard.svelte";
	import LoadingSpinner from "../../components/common/LoadingSpinner.svelte";
	import type { Lobby } from "../../stores/lobby.svelte";
	import { flip } from "svelte/animate";
	import { fade } from "svelte/transition";

	let { lobbies, isLoading = false } = $props();
</script>

<div class="lobbies-list">
	{#if isLoading}
		<div class="status-container" transition:fade>
			<LoadingSpinner text="Loading..." />
		</div>
	{:else if lobbies.length === 0}
		<div class="status-container" transition:fade>
			<p class="empty-text">No lobbies available.</p>
		</div>
	{:else}
		<div class="lobbies-grid">
			{#each lobbies as lobby (lobby.id)}
				<div animate:flip={{ duration: 400 }} transition:fade>
					<LobbyCard {lobby} />
				</div>
			{/each}
		</div>
	{/if}
</div>

<style>
	.lobbies-list { position: relative; min-height: 400px; width: 100%; }
	.status-container {
		position: absolute;
		top: 50px;
		left: 0;
		width: 100%;
		text-align: center;
	}
	.empty-text { font-size: 18px; color: var(--text); opacity: 0.7; }
	.lobbies-grid {
		display: grid;
		grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
		gap: 20px;
	}
</style>