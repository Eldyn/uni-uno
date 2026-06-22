<script lang="ts">
	let {
		src,
		color,
		// Sizing for both the mask and the blended background. Mirrors the
		// per-site mask-size the masked elements used before (e.g. "100% 100%"
		// to stretch, "contain" to fit, "cover" by default).
		fit = "cover"
	}: { src: string; color: string; fit?: string } = $props();
</script>

<div
	class="tinted-sprite"
	style="
		--sprite-img: url('{src}');
		--sprite-color: {color};
		--sprite-fit: {fit};
	"
></div>

<style>
	.tinted-sprite {
		/* Take up 100% of whatever bounding box the parent dictates */
		width: 100%;
		height: 100%;

		/* Presentation & Blend engine */
		background-color: var(--sprite-color);
		background-image: var(--sprite-img);
		background-blend-mode: multiply;

		/* Masking */
		-webkit-mask-image: var(--sprite-img);
		mask-image: var(--sprite-img);

		/* Adaptive Scaling */
		background-size: var(--sprite-fit, cover);
		background-position: center;
		-webkit-mask-size: var(--sprite-fit, cover);
		mask-size: var(--sprite-fit, cover);
		-webkit-mask-position: center;
		mask-position: center;
		-webkit-mask-repeat: no-repeat;
		mask-repeat: no-repeat;
	}
</style>
