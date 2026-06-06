<script lang="ts">
	import { storeLobby } from "../../stores/lobby.svelte";
	import { storeAuth } from "../../stores/auth.svelte";

	let isHost = $derived(storeAuth.username === storeLobby.current?.host);
	let showSettings = $state(false);

	// Valore locale per il timer del turno
	let localTimerValue = $state(
		storeLobby.current?.turn_time_limit_ms ? storeLobby.current.turn_time_limit_ms / 1000 : 15
	);

	// Valore locale per il numero di bot (default a 0)
	let localBotCountValue = $state(
		storeLobby.current?.bot_count !== undefined ? storeLobby.current.bot_count : 0
	);

	// Placeholder per le future regole aggiuntive con le relative descrizioni
	let placeholderRules = $state([
		{
			id: "rule1",
			label: "Rule 1",
			enabled: false,
			desc: "Eldyn"
		},
		{ id: "rule2", label: "Rule 2", enabled: false, desc: "è" },
		{ id: "rule3", label: "Rule 3", enabled: false, desc: "tanto" },
		{ id: "rule4", label: "Rule 4", enabled: false, desc: "molto" },
		{ id: "rule5", label: "Rule 5", enabled: false, desc: "troppo" },
		{ id: "rule6", label: "Rule 6", enabled: false, desc: "gay" }
	]);

	// Sincronizza i valori con lo store quando arrivano aggiornamenti dal server
	$effect(() => {
		if (storeLobby.current?.turn_time_limit_ms) {
			localTimerValue = storeLobby.current.turn_time_limit_ms / 1000;
		}
		if (storeLobby.current?.bot_count !== undefined) {
			localBotCountValue = storeLobby.current.bot_count;
		}
	});

	function togglePublic(e: Event) {
		if (!isHost) return;
		const target = e.target as HTMLInputElement;
		storeLobby.updateSettings({ is_public: target.checked });
	}

	// --- Handlers per il Timer ---
	function handleTimerSliderDrag(e: Event) {
		if (!isHost) return;
		const target = e.target as HTMLInputElement;
		localTimerValue = parseInt(target.value);
	}

	function commitTimerSliderChange() {
		if (!isHost) return;
		storeLobby.updateSettings({ turn_time_limit_ms: localTimerValue * 1000 });
	}

	// --- Handlers per i Bot ---
	function handleBotSliderDrag(e: Event) {
		if (!isHost) return;
		const target = e.target as HTMLInputElement;
		localBotCountValue = parseInt(target.value);
	}

	function commitBotSliderChange() {
		if (!isHost) return;
		storeLobby.updateSettings({ bot_count: localBotCountValue });
	}

	// --- Handler per le regole Placeholder ---
	function handleRuleToggle(index: number, e: Event) {
		if (!isHost) return;
		const target = e.target as HTMLInputElement;
		placeholderRules[index].enabled = target.checked;
	}
</script>

<div class="lobby-settings-container">
	<button
		class="settings-toggle-btn"
		class:active={showSettings}
		onclick={() => (showSettings = !showSettings)}
		aria-expanded={showSettings}
	>
		<span>⚙️ Lobby Settings</span>
		<span class="arrow">{showSettings ? "▲" : "▼"}</span>
	</button>

	{#if showSettings}
		<div class="settings-dropdown">
			<div class="settings-row">
				<label class="toggle-label">
					<input
						type="checkbox"
						checked={storeLobby.current?.is_public ?? false}
						disabled={!isHost}
						onchange={togglePublic}
					/>
					<span>Public Lobby</span>
				</label>
			</div>

			<hr class="settings-divider" />

			<div class="settings-row slider-row">
				<div class="slider-header">
					<label for="turn-timer" class="slider-label">Turn Timer:</label>
					<span class="slider-value">{localTimerValue}s</span>
				</div>
				<input
					id="turn-timer"
					type="range"
					min="5"
					max="30"
					value={localTimerValue}
					disabled={!isHost}
					oninput={handleTimerSliderDrag}
					onchange={commitTimerSliderChange}
					class="custom-slider"
				/>
			</div>

			<hr class="settings-divider" />

			<div class="settings-row slider-row">
				<div class="slider-header">
					<label for="bot-count" class="slider-label">Bot Count:</label>
					<span class="slider-value">{localBotCountValue}</span>
				</div>
				<input
					id="bot-count"
					type="range"
					min="0"
					max="3"
					value={localBotCountValue}
					disabled={!isHost}
					oninput={handleBotSliderDrag}
					onchange={commitBotSliderChange}
					class="custom-slider"
				/>
			</div>

			<hr class="settings-divider" />

			<div class="settings-row rules-row">
				<div class="rules-header">Custom Rules</div>
				<div class="rules-grid">
					{#each placeholderRules as rule, i}
						<label class="toggle-label rule-label" data-tooltip={rule.desc}>
							<input
								type="checkbox"
								checked={rule.enabled}
								disabled={!isHost}
								onchange={(e) => handleRuleToggle(i, e)}
							/>
							<span>{rule.label}</span>
						</label>
					{/each}
				</div>
			</div>
		</div>
	{/if}
</div>

<style>
	.lobby-settings-container {
		margin-bottom: 25px;
		display: inline-block;
		position: relative;
	}

	.settings-toggle-btn {
		display: flex;
		align-items: center;
		gap: 12px;
		padding: 10px 12px;
		background: var(--bg);
		color: var(--text-h);
		border: 2px solid var(--border);
		border-radius: 6px;
		font-size: 14px;
		font-weight: 500;
		cursor: pointer;
		transition: border-color 0.2s;
	}

	.settings-toggle-btn:hover,
	.settings-toggle-btn.active {
		outline: none;
		border-color: var(--accent);
	}

	.settings-toggle-btn .arrow {
		font-size: 10px;
		color: var(--text-h);
		opacity: 0.8;
	}

	.settings-dropdown {
		position: absolute;
		top: calc(100% + 6px);
		left: 0;
		background: var(--bg);
		border: 2px solid var(--border);
		border-radius: 6px;
		padding: 12px 16px;
		min-width: 260px;
		z-index: 10;
		box-shadow: 0 4px 12px rgba(0, 0, 0, 0.4);
		display: flex;
		flex-direction: column;
		gap: 12px;
	}

	.settings-row {
		display: flex;
		flex-direction: column;
		gap: 6px;
	}

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

	.toggle-label input:disabled,
	.toggle-label input:disabled + span {
		cursor: not-allowed;
		opacity: 0.6;
	}

	.settings-divider {
		border: none;
		border-top: 1px solid var(--border);
		margin: 0;
		opacity: 0.5;
	}

	.slider-row {
		gap: 10px;
	}

	.slider-header {
		display: flex;
		justify-content: space-between;
		align-items: center;
	}

	.slider-label {
		font-size: 13px;
		color: var(--text-h);
		font-weight: 500;
	}

	.slider-value {
		font-size: 13px;
		color: var(--accent);
		font-weight: bold;
		font-family: var(--mono);
	}

	/* --- Stili per la griglia delle regole --- */
	.rules-row {
		gap: 10px;
	}

	.rules-header {
		font-size: 13px;
		color: var(--text-h);
		font-weight: 500;
		margin-bottom: 2px;
	}

	.rules-grid {
		display: grid;
		grid-template-columns: 1fr 1fr;
		gap: 8px;
	}

	.rule-label {
		position: relative;
		font-size: 13px;
		cursor: help; /* Il cursore a punto di domanda indica che c'è un info-box */
	}

	.rule-label::after {
		content: attr(data-tooltip);
		position: absolute;
		bottom: 125%; /* Posizionato sopra la riga */
		left: 50%;
		transform: translateX(-50%) scale(0.9);
		padding: 6px 10px;
		background: #1e1e24; /* Sfondo scuro per contrasto */
		color: #ffffff;
		font-size: 11px;
		font-weight: 400;
		border-radius: 4px;
		border: 1px solid var(--border);
		white-space: nowrap;
		z-index: 99;
		box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
		opacity: 0;
		pointer-events: none; /* Evita che dia fastidio ai click */
		transition:
			opacity 0.15s ease,
			transform 0.15s ease;
	}

	.rule-label:hover::after {
		opacity: 1;
		transform: translateX(-50%) scale(1);
	}

	.rule-label input:disabled {
		cursor: not-allowed;
	}

	.custom-slider {
		-webkit-appearance: none;
		width: 100%;
		background: transparent;
		cursor: pointer;
	}

	.custom-slider:disabled {
		cursor: not-allowed;
		opacity: 0.6;
	}

	.custom-slider::-webkit-slider-runnable-track {
		width: 100%;
		height: 6px;
		background: var(--border);
		border-radius: 3px;
	}

	.custom-slider::-webkit-slider-thumb {
		-webkit-appearance: none;
		height: 16px;
		width: 16px;
		border-radius: 50%;
		background: var(--accent);
		margin-top: -5px;
		transition: transform 0.1s;
	}

	.custom-slider::-webkit-slider-thumb:hover {
		transform: scale(1.2);
	}

	.custom-slider:disabled::-webkit-slider-thumb:hover {
		transform: none;
	}

	.custom-slider::-moz-range-track {
		width: 100%;
		height: 6px;
		background: var(--border);
		border-radius: 3px;
	}

	.custom-slider::-moz-range-thumb {
		height: 16px;
		width: 16px;
		border: none;
		border-radius: 50%;
		background: var(--accent);
		transition: transform 0.1s;
	}

	.custom-slider::-moz-range-thumb:hover {
		transform: scale(1.2);
	}
</style>
