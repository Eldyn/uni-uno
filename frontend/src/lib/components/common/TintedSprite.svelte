<script lang="ts">
	export let src: string;
	export let color: string;
</script>

<div
	class="tinted-sprite"
	style="
		--sprite-img: url('{src}');
		--sprite-color: {color};
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
		background-size: cover;
		background-position: center;
		-webkit-mask-size: cover;
		mask-size: cover;
		-webkit-mask-position: center;
		mask-position: center;
		-webkit-mask-repeat: no-repeat;
		mask-repeat: no-repeat;
	}
</style>
