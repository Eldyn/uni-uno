<script lang="ts">
	import { storeNavigation } from "../stores/navigation.svelte";
	import { storeAuth } from "../stores/auth.svelte";
</script>

<div class="screen-container">
	<div class="doodle-bg"></div>

	<header class="logo-container">
		{#each "UNI!" as letter, i}
			<h1 class="logo-char" style="--i: {i}">{letter}</h1>
		{/each}
	</header>

	<main class="button-container">
		{#if !storeAuth.isLoggedIn}
			<button class="pixel-corners" onclick={() => storeNavigation.goto("auth")}> Login </button>
		{:else}
			<div class="logged-in-menu">
				<button class="pixel-corners btn-wide" onclick={() => storeNavigation.goto("lobbies")}>
					Entra in Stanza
				</button>
				<div class="secondary-buttons">
					<button class="pixel-corners" onclick={() => storeNavigation.goto("stats")}>
						Stats
					</button>
					<button class="pixel-corners" onclick={() => storeAuth.logout()}>Logout</button>
				</div>
			</div>
		{/if}
	</main>
</div>

<style>
	.doodle-bg {
		position: fixed;
		inset: 0;
		background-image: url("/assets/bg_main.png");
		background-size: cover;
		z-index: 0;
	}

	.screen-container {
		position: relative;
		display: flex;
		flex-direction: column;
		justify-content: flex-end;
		align-items: center;

		gap: 2rem;
		min-height: 100vh;
		padding-bottom: 6rem;
		background: var(--bg);
		overflow: hidden;

		-webkit-font-smoothing: none;
		-moz-osx-font-smoothing: grayscale;
		font-smooth: never;
	}

	@keyframes fallAndFade {
		0% {
			transform: translateY(0);
			opacity: 0;
		}
		8% {
			opacity: 0.45;
		}
		92% {
			opacity: 0.45;
		}
		100% {
			transform: translateY(125vh);
			opacity: 0;
		}
	}

	@keyframes spin3D {
		0% {
			transform: rotateX(var(--initial-rx)) rotateY(0deg) rotateZ(var(--rotate-z))
				scale(var(--scale));
		}
		100% {
			transform: rotateX(calc(var(--initial-rx) + 40deg)) rotateY(720deg)
				rotateZ(calc(var(--rotate-z) + 150deg)) scale(var(--scale));
		}
	}

	.logo-container,
	.button-container {
		position: relative;
		z-index: 1;
	}

	.logo-container {
		display: flex;
		justify-content: center;
		color: #ffffff;

		margin: 0;
		text-align: center;
		display: flex;
		align-items: center;
		justify-content: center;
		gap: 0.4rem;
	}

	.logo-char {
		font-size: 10rem;
		display: block;
		-webkit-text-stroke: 2px #1a1a1a;
		will-change: transform;
		animation: waveBounce 1s ease-in-out infinite;
		animation-delay: calc(var(--i) * 0.15s);
	}

	/* Animazione a rimbalzo singolo carattere */
	@keyframes waveBounce {
		0%,
		100% {
			transform: translateY(0);
			text-shadow: 5px 5px 0px #1a1a1a;
		}
		50% {
			transform: translateY(-20px); /* Push upwards */
			text-shadow: 5px 25px 0px #1a1a1a; /* The shadow stretches downwards */
		}
	}

	.button-container {
		display: flex;
		justify-content: center;
		align-items: center;
		margin-bottom: 3rem;
	}

	.logged-in-menu {
		display: flex;
		flex-direction: column;
		gap: 1.5rem;
	}

	.secondary-buttons {
		display: flex;
		justify-content: center;
		gap: 1.5rem;
	}

	.btn-wide {
		width: 100%;
	}

	button {
		padding: 1.5rem 2.5rem;
		font-size: 1.35rem;
		font-family: "Pixel", sans-serif;
		font-weight: 900;
		letter-spacing: 1.5px;

		line-height: 1.3;

		color: #ffffff;
		background-color: #8d42d3;

		cursor: pointer;
		box-shadow: 5px 5px 0px #1a1a1a;
		transition:
			transform 0.08s ease,
			box-shadow 0.08s ease,
			background-color 0.2s ease;
	}

	button:hover {
		background-color: #9c4fe3;
		transform: translate(-3px, -3px);
		box-shadow: 8px 8px 0px #1a1a1a;
	}

	button:active {
		transform: translate(5px, 5px);
		box-shadow: 0px 0px 0px #1a1a1a;
	}
</style>
