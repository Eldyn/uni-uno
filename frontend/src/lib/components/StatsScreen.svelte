<script lang="ts">
	import { onMount } from "svelte";
	import { storeStats } from "../stores/stats.svelte";
	import { storeNavigation } from "../stores/navigation.svelte";

	onMount(() => {
		storeStats.fetchMe();
		storeStats.fetchLeaderboard();
	});

	function getWinRate(wins: number, losses: number): string {
		const total = wins + losses;
		if (total === 0) return "0%";
		return Math.round((wins / total) * 100) + "%";
	}
</script>

<div class="stats-container">
	<header class="top-bar">
		<button class="back-btn" onclick={() => storeNavigation.back()}> ← Back </button>
		<h1>Global Leaderboard</h1>
		<div style="width: 80px;"></div>
	</header>

	<div class="content">
		{#if storeStats.myStats}
			<div class="my-stats-card">
				<div class="profile-section">
					<h2>{storeStats.myStats.username}</h2>
					<span class="rank-badge">
						{storeStats.myStats.rank ? `#${storeStats.myStats.rank} Global` : "Unranked"}
					</span>
				</div>
				<div class="stats-grid">
					<div class="stat-box">
						<span class="stat-label">Wins</span>
						<span class="stat-value text-green">{storeStats.myStats.total_wins}</span>
					</div>
					<div class="stat-box">
						<span class="stat-label">Losses</span>
						<span class="stat-value text-red">{storeStats.myStats.total_losses}</span>
					</div>
					<div class="stat-box">
						<span class="stat-label">Win Rate</span>
						<span class="stat-value text-blue">
							{getWinRate(storeStats.myStats.total_wins, storeStats.myStats.total_losses)}
						</span>
					</div>
				</div>
			</div>
		{/if}

		<div class="leaderboard-section">
			<h3>Top 50 Players</h3>

			{#if storeStats.isLoading}
				<p class="loading-text">Loading rankings...</p>
			{:else if storeStats.leaderboard.length === 0}
				<p class="loading-text">No games have been played yet!</p>
			{:else}
				<div class="leaderboard-list">
					{#each storeStats.leaderboard as player, index}
						<div
							class="leaderboard-row"
							class:is-me={player.username === storeStats.myStats?.username}
						>
							<div class="rank-col">
								{#if index === 0}
									🥇
								{:else if index === 1}
									🥈
								{:else if index === 2}
									🥉
								{:else}
									#{player.rank}
								{/if}
							</div>
							<div class="name-col">{player.username}</div>
							<div class="score-col">
								<span class="wins">{player.total_wins}W</span> -
								<span class="losses">{player.total_losses}L</span>
							</div>
						</div>
					{/each}
				</div>
			{/if}
		</div>
	</div>
</div>

<style>
	.stats-container {
		display: flex;
		flex-direction: column;
		height: 100vh;
		background-color: var(--bg);
		color: white;
	}

	.top-bar {
		display: flex;
		justify-content: space-between;
		align-items: center;
		padding: 20px;
		background-color: #1c1c1e;
		border-bottom: 2px solid #333;
	}

	.top-bar h1 {
		margin: 0;
		font-size: 1.5rem;
		color: #ffcc00;
	}

	.back-btn {
		background: transparent;
		color: #ffcc00;
		border: 1px solid #ffcc00;
		padding: 8px 16px;
		border-radius: 8px;
		cursor: pointer;
		font-weight: bold;
		transition: all 0.2s;
	}

	.back-btn:hover {
		background: rgba(255, 204, 0, 0.1);
	}

	.content {
		flex: 1;
		overflow-y: auto;
		padding: 20px;
		max-width: 800px;
		margin: 0 auto;
		width: 100%;
		display: flex;
		flex-direction: column;
		gap: 30px;
	}

	/* --- My Stats Card --- */
	.my-stats-card {
		background: linear-gradient(145deg, #2a2a2d, #1f1f22);
		border-radius: 16px;
		padding: 24px;
		box-shadow: 0 8px 20px rgba(0, 0, 0, 0.4);
		border: 1px solid #3a3a3d;
	}

	.profile-section {
		display: flex;
		justify-content: space-between;
		align-items: center;
		margin-bottom: 20px;
		border-bottom: 1px solid #444;
		padding-bottom: 15px;
	}

	.profile-section h2 {
		margin: 0;
		font-size: 1.8rem;
	}

	.rank-badge {
		background: #ffcc00;
		color: #000;
		padding: 6px 14px;
		border-radius: 20px;
		font-weight: 900;
		font-size: 1.1rem;
	}

	.stats-grid {
		display: flex;
		justify-content: space-around;
		text-align: center;
	}

	.stat-box {
		display: flex;
		flex-direction: column;
		gap: 8px;
	}
	.stat-label {
		color: #8e8e93;
		font-size: 0.9rem;
		text-transform: uppercase;
		letter-spacing: 1px;
	}
	.stat-value {
		font-size: 2rem;
		font-weight: bold;
	}
	.text-green {
		color: #34c759;
	}
	.text-red {
		color: #ff3b30;
	}
	.text-blue {
		color: #007aff;
	}

	/* --- Leaderboard List --- */
	.leaderboard-section {
		background: #1c1c1e;
		border-radius: 16px;
		padding: 20px;
		box-shadow: 0 4px 15px rgba(0, 0, 0, 0.3);
	}

	.leaderboard-section h3 {
		margin-top: 0;
		color: #ffcc00;
		margin-bottom: 15px;
	}

	.leaderboard-list {
		display: flex;
		flex-direction: column;
		gap: 8px;
	}

	.leaderboard-row {
		display: flex;
		align-items: center;
		background: #2a2a2d;
		padding: 12px 16px;
		border-radius: 10px;
		transition: transform 0.1s;
	}

	.leaderboard-row:hover {
		transform: scale(1.01);
		background: #323236;
	}

	.leaderboard-row.is-me {
		border: 2px solid #007aff;
		background: rgba(0, 122, 255, 0.1);
	}

	.rank-col {
		width: 50px;
		font-weight: bold;
		color: #8e8e93;
		font-size: 1.2rem;
	}
	.name-col {
		flex: 1;
		font-weight: 600;
		font-size: 1.1rem;
	}
	.score-col {
		font-weight: bold;
		font-size: 1.1rem;
	}
	.wins {
		color: #34c759;
	}
	.losses {
		color: #ff3b30;
	}

	.loading-text {
		text-align: center;
		color: #8e8e93;
		font-style: italic;
	}
</style>
