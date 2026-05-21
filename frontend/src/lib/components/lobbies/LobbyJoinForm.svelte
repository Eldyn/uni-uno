<script lang="ts">
    import { toastStore } from "$stores/ui.svelte";
    import { emitAndWait, ServerAction, ClientAction } from "$lib/ws.svelte";
    import { gameStore } from "$stores/game.svelte";
    import type { Lobby } from "$stores/game.svelte";

    let roomCodeInput = $state("");
    let roomCodeError = $state("");
    let isLoading = $state(false);

    let { onJoinSuccess }: { onJoinSuccess: (inviteCode: string) => void } =
        $props();

    function validateForm(): boolean {
        roomCodeError = "";

        const code = roomCodeInput.trim();
        if (!code) {
            roomCodeError = "Invite code is required.";
            return false;
        }

        if (code.length !== 6 || !/^[A-Z0-9]+$/.test(code)) {
            roomCodeError = "Invite code must be 6 alphanumeric characters.";
            return false;
        }

        return true;
    }

    async function handleSubmit(event: SubmitEvent) {
        event.preventDefault();

        if (!validateForm()) {
            return;
        }

        isLoading = true;

        try {
            const response = await emitAndWait(
                ClientAction.LobbyJoin,
                { code: roomCodeInput.toUpperCase() },
                ServerAction.LobbyJoined,
                5000,
            );

            const inviteCode = response.invite_code as string;
            const members = (response.members as unknown[]) || [];

            // Handle successful lobby join
            if (response.invite_code && response.lobby_id) {
                // Store invite code in sessionStorage for reconnection
                sessionStorage.setItem("lobby_code", inviteCode);

                // Update game store with current lobby
                const lobby: Lobby = {
                    lobby_id: String(response.lobby_id),
                    name: `Joined lobby`,
                    host: "", // Will get from response members if needed
                    players: members,
                    player_count: 1,
                    max_players: 4,
                    // status: "waiting" as const,
                    // createdAt: Date.now(),
                };
                gameStore.currentLobby = lobby;

                toastStore.showSuccess("Joined lobby successfully!");
                onJoinSuccess(inviteCode);
                roomCodeInput = "";
            }
        } catch (error) {
            const errorMsg =
                error instanceof Error ? error.message : String(error);
            if (errorMsg.includes("Lobby not found")) {
                roomCodeError = "Lobby not found. Check your invite code.";
            } else if (errorMsg.includes("full")) {
                toastStore.showError("This lobby is full.");
            } else if (errorMsg.includes("Already")) {
                toastStore.showError("You are already a member of this lobby.");
            } else {
                toastStore.showError(
                    "Failed to join lobby. Check your connection.",
                );
            }
        } finally {
            isLoading = false;
        }
    }
</script>

<form onsubmit={handleSubmit} class="join-lobby-form">
    <h3>Join Lobby</h3>

    <div class="form-group">
        <label for="invite-code">Invite Code:</label>
        <input
            id="invite-code"
            type="text"
            bind:value={roomCodeInput}
            placeholder="Enter 6-character invite code"
            disabled={isLoading}
            class:error={roomCodeError}
            maxlength="6"
        />
        {#if roomCodeError}
            <span class="error-text">{roomCodeError}</span>
        {/if}
    </div>

    <button type="submit" disabled={isLoading} class="submit-button">
        {isLoading ? "Joining..." : "Join Lobby"}
    </button>
</form>

<style>
    .join-lobby-form {
        display: flex;
        flex-direction: column;
        gap: 16px;
    }

    h3 {
        margin: 0;
        font-size: 16px;
        color: var(--text-h);
    }

    .form-group {
        display: flex;
        flex-direction: column;
        gap: 6px;
    }

    label {
        font-size: 14px;
        font-weight: 500;
        color: var(--text-h);
    }

    input {
        padding: 10px 12px;
        border: 2px solid var(--border);
        border-radius: 6px;
        font-size: 14px;
        color: var(--text-h);
        background: var(--bg);
        transition: border-color 0.2s;
    }

    input:focus {
        outline: none;
        border-color: var(--accent);
    }

    input:disabled {
        opacity: 0.6;
        cursor: not-allowed;
    }

    input.error {
        border-color: #dc2626;
    }

    .error-text {
        font-size: 12px;
        color: #dc2626;
    }

    .submit-button {
        padding: 10px 16px;
        background: var(--accent);
        color: white;
        border: none;
        border-radius: 6px;
        font-size: 14px;
        font-weight: 500;
        cursor: pointer;
        transition: opacity 0.2s;
    }

    .submit-button:hover:not(:disabled) {
        opacity: 0.9;
    }

    .submit-button:disabled {
        opacity: 0.6;
        cursor: not-allowed;
    }
</style>
