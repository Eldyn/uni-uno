<script lang="ts">
	import { type Snippet } from "svelte";

	interface Props {
		children: Snippet; // The trigger element (e.g., the item icon)
		tooltipContent: Snippet; // The floating item lore / text
	}

	let { children, tooltipContent } = $props<Props>();

	let isVisible = $state(false);
	let mouseX = $state(0);
	let mouseY = $state(0);

	let shiftX = $state(0); // 0 = right of cursor, 1 = left of cursor
	let shiftY = $state(0); // 0 = below cursor, 1 = above cursor

	function handleMouseMove(e: MouseEvent) {
		mouseX = e.clientX;
		mouseY = e.clientY;

		const windowWidth = window.innerWidth;
		const windowHeight = window.innerHeight;

		const safetyZoneX = 350;
		const safetyZoneY = 200;

		if (mouseX + safetyZoneX > windowWidth) {
			shiftX = 1;
		} else {
			shiftX = 0;
		}

		if (mouseY + safetyZoneY > windowHeight) {
			shiftY = 1;
		} else {
			shiftY = 0;
		}
	}
</script>

<div
	class="tooltip-container"
	role="tooltip"
	onmouseenter={() => (isVisible = true)}
	onmouseleave={() => (isVisible = false)}
	onmousemove={handleMouseMove}
>
	{@render children()}

	{#if isVisible}
		<div
			class="minecraft-floating-tooltip"
			style:--mouse-x="{mouseX}px"
			style:--mouse-y="{mouseY}px"
			style:--shift-x={shiftX ? "-100%" : "15px"}
			style:--shift-y={shiftY ? "-100%" : "15px"}
		>
			{@render tooltipContent()}
		</div>
	{/if}
</div>

<style>
	.tooltip-container {
		display: inline-block;
		cursor: pointer;
	}

	.minecraft-floating-tooltip {
		position: fixed;
		top: var(--mouse-y);
		left: var(--mouse-x);

		transform: translate(var(--shift-x), var(--shift-y));

		pointer-events: none;
		z-index: 9999;

		background: rgba(16, 1, 16, 0.94);
		border: 2px solid #2e0664;
		outline: 2px solid #100110;
		padding: 10px 12px;
		border-radius: 4px;
		box-shadow: 0 4px 10px rgba(0, 0, 0, 0.5);
		font-family: "JetBrainsMono", monospace;
		white-space: nowrap;
	}
</style>
