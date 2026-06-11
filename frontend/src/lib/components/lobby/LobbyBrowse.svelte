<script lang="ts">
	import LobbyList from "./LobbyList.svelte";
	import LobbyCreateForm from "./LobbyCreateForm.svelte";
	import LobbyJoinForm from "./LobbyJoinForm.svelte";

	import { onMount } from "svelte";
	import { storeAuth } from "../../stores/auth.svelte";
	import { storeLobby } from "../../stores/lobby.svelte";
	import { storeNavigation } from "../../stores/navigation.svelte";

	let refreshing = $state(false);

	onMount(() => {
		storeLobby.fetchList();
	});

	async function handleRefresh() {
		refreshing = true;
		await storeLobby.fetchList();
		refreshing = false;
	}
</script>

<div class="lobbies-screen">
	<!-- HEADER PRINCIPALE -->
	<header class="lobbies-header">
		<div class="header-left">
			<p class="user-info">Welcome <span class="mono">{storeAuth.username}</span></p>
		</div>

		<div class="main-title-container">
			<h1 class="main-title-text">Lobbies</h1>
		</div>

		<div class="header-actions">
			<button type="button" class="pixel-btn" onclick={() => storeNavigation.goto("main")}>
				Back
			</button>
			<button type="button" class="pixel-btn logout" onclick={() => storeAuth.logout()}>
				Logout
			</button>
		</div>
	</header>

	<div class="main-layout">
		<!-- PARTE SINISTRA: LISTA -->
		<section class="lobbies-section">
			<!-- Area della lista (sopra) -->
			<div class="lobbies-scroll-area">
				<LobbyList lobbies={storeLobby.available} isLoading={storeLobby.isLoadingList} />
			</div>

			<!-- Tasto Refresh (spostato in basso) -->
			<div class="controls">
				<button
					type="button"
					class="pixel-btn refresh"
					onclick={handleRefresh}
					disabled={refreshing}
				>
					{refreshing ? "⟳ Refreshing..." : "⟳ Refresh"}
				</button>
			</div>
		</section>

		<!-- SIDEBAR DESTRA -->
		<aside class="sidebar">
			<!-- Sezione Create -->
			<div class="sidebar-section">
				<div class="sidebar-form-box">
					<h2 class="sidebar-form-title">Create Lobby</h2>
				</div>
				<div class="sidebar-content">
					<LobbyCreateForm />
				</div>
			</div>

			<div class="sidebar-divider"></div>

			<!-- Sezione Join -->
			<div class="sidebar-section">
				<div class="sidebar-form-box">
					<h2 class="sidebar-form-title">Join Lobby</h2>
				</div>
				<div class="sidebar-content">
					<LobbyJoinForm />
				</div>
			</div>
		</aside>
	</div>
</div>

<style>
	/* --- SETUP GENERALE (NO SCROLL) --- */
	:global(body) {
		margin: 0;
		overflow: hidden;
		overscroll-behavior: none;
	}

	.lobbies-screen {
		position: fixed;
		inset: 0;
		background-image: url("/assets/bg_main.png");
		background-size: cover;
		z-index: 0;
		display: flex;
		flex-direction: column;
		height: 100vh;
		width: 100vw;
	}

	/* --- HEADER --- */
	.lobbies-header {
		display: grid;
		grid-template-columns: 1fr auto 1fr;
		align-items: center;
		padding: 0 40px;
		height: 120px;
		border-bottom: 2px solid var(--border);
		background: var(--bg);
		flex-shrink: 0;
	}

	.main-title-text {
		font-family: "FatPixel", sans-serif;
		font-size: 40px;
		color: var(--text-h);
		margin: 0;
		text-transform: uppercase;
		line-height: 1;
		text-align: center;
		letter-spacing: 2px;
	}

	/* --- BOTTONE PIXEL --- */
	.pixel-btn {
		padding: 15px;
		background: var(--accent);
		color: white;
		border: none;
		font-family: "Pixel", sans-serif;
		font-size: 20px;
		cursor: pointer;
		text-transform: uppercase;
		transition:
			filter 0.2s,
			transform 0.1s;
		display: inline-flex;
		align-items: center;
		justify-content: center;
	}

	.pixel-btn:hover:not(:disabled) {
		filter: brightness(1.2);
	}

	.pixel-btn:active:not(:disabled) {
		transform: scale(0.95);
	}

	.pixel-btn:disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.pixel-btn.logout {
		background: #e91e63;
	}

	/* --- SIDEBAR --- */
	.sidebar-form-box {
		width: 100%;
		padding: 45px 20px;
		background: rgb(20, 20, 20);
		border-bottom: 2px solid var(--border);
		display: flex;
		justify-content: center;
		align-items: center;
	}

	.sidebar-form-title {
		font-family: "FatPixel", sans-serif;
		font-size: 15px;
		color: var(--text-h);
		margin: 0;
		text-transform: uppercase;
	}

	.main-layout {
		display: flex;
		flex: 1;
		overflow: hidden;
	}

	.lobbies-section {
		flex: 1;
		padding: 30px;
		display: flex;
		flex-direction: column;
	}

	.lobbies-scroll-area {
		flex: 1;
		overflow-y: auto;
	}

	.sidebar {
		width: 400px;
		background: var(--bg);
		border-left: 2px solid var(--border);
		display: flex;
		flex-direction: column;
	}

	.sidebar-content {
		padding: 40px 25px;
	}

	.sidebar-divider {
		height: 4px;
		background: var(--border);
		width: 100%;
		flex-shrink: 0;
	}

	/* --- ELEMENTI ACCESSORI --- */
	.user-info {
		font-size: 18px;
		color: white;
		margin: 0;
	}
	.mono {
		font-family: "Pixel", monospace;
		color: var(--accent);
	}
	.header-actions {
		display: flex;
		gap: 12px;
		justify-content: flex-end;
	}

	/* MODIFICATO PER STARE IN BASSO */
	.controls {
		margin-top: 20px;
		display: flex;
		justify-content: flex-start;
	}

	/* --- RESPONSIVE --- */
	@media (max-width: 1100px) {
		.main-layout {
			flex-direction: column;
			overflow-y: auto;
		}
		.sidebar {
			width: 100%;
			border-left: none;
			border-top: 4px solid var(--border);
		}
		.lobbies-header {
			grid-template-columns: 1fr;
			height: auto;
			padding: 20px;
			gap: 20px;
		}
		.header-actions {
			justify-content: center;
		}
	}
</style>