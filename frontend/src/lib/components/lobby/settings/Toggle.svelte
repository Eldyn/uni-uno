<script lang="ts">
	let {
		label,
		checked,
		disabled = false,
		oncommit
	}: {
		label: string;
		checked: boolean;
		disabled?: boolean;
		oncommit: (value: boolean) => void;
	} = $props();
</script>

<label class="toggle-label" class:disabled>
	<input
		type="checkbox"
		{checked}
		{disabled}
		onchange={(e) => oncommit((e.target as HTMLInputElement).checked)}
	/>
	<span>{label}</span>
</label>

<style>
	.toggle-label {
		display: flex;
		align-items: center;
		gap: 8px;
		font-size: 14px;
		font-weight: 500;
		color: var(--text-h);
		cursor: pointer;
		user-select: none;
	}

	.toggle-label.disabled {
		cursor: not-allowed;
		opacity: 0.6;
	}

	.toggle-label input:disabled + span {
		cursor: not-allowed;
	}
</style>
