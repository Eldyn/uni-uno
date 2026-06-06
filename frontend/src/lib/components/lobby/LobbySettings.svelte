<script lang="ts">
	import { storeLobby } from "../../stores/lobby.svelte";
	import { storeAuth } from "../../stores/auth.svelte";
	import Tooltip from "../common/Tooltip.svelte";

	let isHost = $derived(storeAuth.username === storeLobby.current?.host);
	let showSettings = $state(false);

	// Valore locale per il timer del turno
	let localTimerValue = $state(
		storeLobby.current?.settings.turn_time_limit_ms
			? storeLobby.current.settings.turn_time_limit_ms / 1000
			: 15
	);

	let localIsPublic = $state(storeLobby.current?.settings.is_public);

	// Valore locale per il numero di bot
	let localBotCountValue = $state(
		storeLobby.current?.settings.bot_count !== undefined ? storeLobby.current.settings.bot_count : 0
	);

	let customRules = $state([
		{ id: "rule1", label: "Rule 1", description: "Test 1", enabled: false },
		{ id: "rule2", label: "Rule 2", description: "Test 2", enabled: false },
		{ id: "rule3", label: "Rule 3", description: "Test 3", enabled: false },
		{ id: "rule4", label: "Rule 4", description: "Test 4", enabled: false },
		{ id: "rule5", label: "Rule 5", description: "Test 5", enabled: false },
		{ id: "rule6", label: "Rule 6", description: "Test 6", enabled: false }
	]);

	// Sincronizza i valori con lo store in totale sicurezza
	$effect(() => {
		if (storeLobby.current?.settings.turn_time_limit_ms !== undefined) {
			localTimerValue = storeLobby.current.settings.turn_time_limit_ms / 1000;
		}
		if (storeLobby.current?.settings.bot_count !== undefined) {
			localBotCountValue = storeLobby.current.settings.bot_count;
		}

		if (storeLobby.current?.settings.is_public !== undefined) {
			localIsPublic = storeLobby.current.settings.is_public;
		}

		// TODO: per far funzionare sta roba serve implementare bene le regole
		// CONTROLLO CRASH: Sovrascrivi i dati locali SOLO se lo store contiene un array reale
		// if (
		// 	storeLobby.current?.settings.active_mods &&
		// 	Array.isArray(storeLobby.current.settings.active_mods)
		// ) {
		//  customRules = storeLobby.current.settings.active_mods;
		// }
	});

	function handlePublic(e: Event) {
		if (!isHost) return;
		const target = e.target as HTMLInputElement;

		// 1. Update the local state immediately so the UI reacts
		localIsPublic = target.checked;

		// 2. Send the newly updated state to the server
		storeLobby.updateSettings({ is_public: localIsPublic });
	}

	function handleTimerSliderDrag(e: Event) {
		if (!isHost) return;
		const target = e.target as HTMLInputElement;
		localTimerValue = parseInt(target.value, 10);
	}

	function commitTimerSliderChange() {
		if (!isHost) return;
		storeLobby.updateSettings({ turn_time_limit_ms: localTimerValue * 1000 });
	}

	function handleBotSliderDrag(e: Event) {
		if (!isHost) return;
		const target = e.target as HTMLInputElement;
		localBotCountValue = parseInt(target.value, 10);
	}

	function commitBotSliderChange() {
		if (!isHost) return;
		storeLobby.updateSettings({ bot_count: localBotCountValue });
	}

	function handleRuleToggle(index: number, e: Event) {
		if (!isHost) return;
		const target = e.target as HTMLInputElement;
		customRules[index].enabled = target.checked;

		// Invia l'aggiornamento allo store
		// storeLobby.updateSettings({ custom_rules: customRules });
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
						checked={localIsPublic}
						disabled={!isHost}
						onchange={handlePublic}
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
					{#each customRules as rule, i}
						<Tooltip>
							{#snippet tooltipContent()}
								<span>{rule.description ?? "Nessuna descrizione disponibile."}</span>
							{/snippet}

							<label class="toggle-label rule-label">
								<input
									type="checkbox"
									checked={rule.enabled}
									disabled={!isHost}
									onchange={(e) => handleRuleToggle(i, e)}
								/>
								<span>{rule.label}</span>
							</label>
						</Tooltip>
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
		min-width: 240px;
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
		cursor: help;
		font-size: 13px;
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
