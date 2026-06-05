<script lang="ts">
    import { storeGame } from "../../stores/game.svelte";
    import { ClientAction, ws } from "../../stores/ws.svelte";

    // Calcola reattivamente se il giocatore ha almeno una carta giocabile in mano
    let hasPlayableCard = $derived(
        storeGame.localPlayer?.hand?.some(card => 
            card.color === "wild" || 
            card.color === storeGame.state?.active_color || 
            (storeGame.state?.top_card && card.value === storeGame.state.top_card.value)
        ) ?? false
    );
</script>

{#if storeGame.localPlayer && storeGame.localPlayer.hand?.length === 2 && storeGame.state?.current_turn === storeGame.localPlayer.username && hasPlayableCard}
    {#if !storeGame.localPlayer.has_called_uno}
        <button
            type="button"
            class="uno-floating-button animate-pulse"
            onclick={() => {
                ws.emit(ClientAction.GameCallUno);
                console.log(ClientAction.GameCallUno);
            }}
        >
            🔥 UNO!
        </button>
    {:else}
        <div class="uno-floating-badge">✅ UNO Dichiarato</div>
    {/if}
{/if}

<style>
    .uno-floating-button {
        position: fixed;
        bottom: 24px;
        right: 24px;
        z-index: 150;
        padding: 16px 28px;
        background: linear-gradient(135deg, #ff416c, #ff4b2b);
        color: white;
        border: none;
        border-radius: 50px;
        font-size: 18px;
        font-weight: 900;
        letter-spacing: 1px;
        cursor: pointer;
        box-shadow: 0 8px 25px rgba(255, 75, 43, 0.5);
        transition:
            transform 0.1s,
            box-shadow 0.2s;
    }

    .uno-floating-button:hover {
        transform: scale(1.08);
        box-shadow: 0 12px 30px rgba(255, 75, 43, 0.7);
    }

    .uno-floating-button:active {
        transform: scale(0.95);
    }

    .uno-floating-badge {
        position: fixed;
        bottom: 24px;
        right: 24px;
        z-index: 150;
        padding: 10px 20px;
        background: #018d41;
        color: white;
        border-radius: 30px;
        font-size: 14px;
        font-weight: bold;
        text-transform: uppercase;
        box-shadow: 0 4px 12px rgba(1, 141, 65, 0.4);
    }

    @keyframes pulse {
        0% {
            transform: scale(1);
        }
        50% {
            transform: scale(1.04);
        }
        100% {
            transform: scale(1);
        }
    }

    .animate-pulse {
        animation: pulse 1.2s infinite ease-in-out;
    }
</style>