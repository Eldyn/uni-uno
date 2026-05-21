<script lang="ts">
    import { validateRoomName } from "$utils/validation";
    import { emitAndWait } from "$lib/ws.svelte";
    import { ClientAction, ServerAction } from "$lib/ws.svelte";
    import { gameStore } from "$stores/game.svelte";
    import type { Lobby } from "$lib/stores/game.svelte";
    import { toastStore } from "$lib/stores/ui.svelte";

    let roomNameInput = $state("");
    let roomNameError = $state("");
    let isLoading = $state(false);

    let { onCreateSuccess }: { onCreateSuccess: (inviteCode: string) => void } =
        $props();

    function validateForm(): boolean {
        roomNameError = "";

        const roomValidation = validateRoomName(roomNameInput);
        if (!roomValidation.valid) {
            roomNameError = roomValidation.error || "";
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
                ClientAction.LobbyCreate,
                {},
                ServerAction.LobbyJoined,
                5000,
            );

            const inviteCode = response.invite_code as string;
            const members = (response.members as unknown[]) || [];

            // Handle successful lobby creation
            if (response.invite_code && response.lobby_id) {
                // Store invite code in sessionStorage for reconnection
                sessionStorage.setItem("lobby_code", inviteCode);

                // Update game store with current lobby
                const lobby: Lobby = {
                    lobby_id: String(response.lobby_id),
                    name: roomNameInput,
                    host: "", // Will get from response members if needed
                    players: members,
                    max_players: 4,
                    player_count: 1,
                    // status: "waiting" as const,
                    // createdAt: Date.now(),
                };

                gameStore.currentLobby = lobby;

                toastStore.showSuccess("Lobby created successfully!");
                onCreateSuccess(inviteCode);
                roomNameInput = "";
            }
        } catch (error) {
            toastStore.showError(
                "Failed to create lobby. Check your connection.",
            );
        } finally {
            isLoading = false;
        }
    }
</script>

<form onsubmit={handleSubmit} class="create-lobby-form">
    <h3>Create New Lobby</h3>

    <div class="form-group">
        <label for="room-name">Room Name:</label>
        <input
            id="room-name"
            type="text"
            bind:value={roomNameInput}
            placeholder="Enter room name"
            disabled={isLoading}
            class:error={roomNameError}
        />
        {#if roomNameError}
            <span class="error-text">{roomNameError}</span>
        {/if}
    </div>

    <button type="submit" disabled={isLoading} class="submit-button">
        {isLoading ? "Creating..." : "Create Lobby"}
    </button>
</form>

<style>
    .create-lobby-form {
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

    input,
    select {
        padding: 10px 12px;
        border: 2px solid var(--border);
        border-radius: 6px;
        font-size: 14px;
        color: var(--text-h);
        background: var(--bg);
        transition: border-color 0.2s;
    }

    input:focus,
    select:focus {
        outline: none;
        border-color: var(--accent);
    }

    input:disabled,
    select:disabled {
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
