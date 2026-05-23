<script lang="ts">
	import { gameStore } from "$lib/stores/game.svelte";
	import { navigationStore, toastStore } from "$lib/stores/ui.svelte";
	import { ClientAction, ws } from "$lib/ws.svelte";

	console.log(gameStore.state.currentLobby);

	// BUG: This currently crashes the server if the client then attempts a rejoin
	async function leaveLobby() {
		const response = await ws.emitAndWait(ClientAction.LobbyLeave);

		if (!response.ok) {
			toastStore.showError(response.reason);
			return;
		}

		gameStore.currentLobby = null;
		localStorage.removeItem("lobby_code");
		navigationStore.screen = "lobbies";
	}
</script>

<div>
	<button onclick={leaveLobby}>Leave</button>
	<h1>{gameStore.state.currentLobby?.host}</h1>
	<ul>
		{#each gameStore.state.currentLobby?.members as member}
			<li>{member.username} [host: {member.is_host}] [connected: {member.is_connected}]</li>
		{/each}
	</ul>
</div>

<style>
</style>
