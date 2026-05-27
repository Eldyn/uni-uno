<script lang="ts">
	import { storeLobby } from "../../stores/lobby.svelte";
</script>

<link rel="preload" href="fonts/JetBrainsMono.woff2" as="font" type="font/woff2" />

<div>
	<button class="join-button" onclick={storeLobby.leave}>Leave</button>
	<h1>{`${storeLobby.current!.name} | ${storeLobby.current!.invite_code}`}</h1>
	<ul class="memberlist">
		{#each storeLobby.current!.members as member}
			<li class="member">
				{member.username}
				{#if member.is_host}
					<span style="color: lightgoldenrodyellow"> 󱟜 </span>
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

	.join-button {
		padding: 8px 16px;
		background: var(--accent);
		color: white;
		border: none;
		border-radius: 6px;
		font-size: 14px;
		font-weight: 500;
		cursor: pointer;
		transition: opacity 0.2s;
	}

	.memberlist {
		list-style-type: none;
	}

	.member {
		font-family: "JetBrainsMono";
		color: lightgray;
	}

	.off {
		color: salmon;
	}

	.on {
		color: lightgreen;
	}
</style>
