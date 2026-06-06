<script lang="ts">
    import { storeGame } from "../../stores/game.svelte";
    import { fly } from "svelte/transition";

    let { gameColor = "green" }: { gameColor?: string } = $props();

    function handleDrawClick() {
        if (storeGame.state?.current_turn === storeGame.localPlayer?.username) {
            storeGame.drawCard();
        }
    }

    function handleCardClick(cardId: number) {
        storeGame.playCard(cardId);
    }

    let opponents = $derived(
        storeGame.state?.players.filter((p) => p.username !== storeGame.localPlayer?.username) ?? []
    );

    let leftPlayer = $derived(opponents[0]);
    let topPlayer = $derived(opponents[1]);
    let rightPlayer = $derived(opponents[2]);
</script>

<div class="sfondo-gioco"></div>

<div class="game-field perspective {gameColor}">
    <div id="player" style="position: relative; width: 100%; height: 100%;">
        <div
            style="position: absolute; top: 0px; width: 100%; text-align: center; font-weight: bold; color: white;"
        >
            (You) {storeGame.localPlayer?.username || ""}
        </div>
        <div
            class="player_hand"
            style="
            position: absolute; top: 50%; left: 50%;
            transform: translate(-50%, -30%);
            width: calc({storeGame.localPlayer?.hand?.length ?? 0} * 2.2em + 7.2em);
            height: calc(var(--cardSize) * 1.5357);
        "
        >
            {#each storeGame.localPlayer?.hand ?? [] as card, i (card.id)}
                <div
                    class="card {card.color}"
                    role="button"
                    tabindex="0"
                    onclick={() => handleCardClick(card.id)}
                    onkeydown={(e) => e.key === "Enter" && handleCardClick(card.id)}
                    in:fly={{ y: -300, x: -100, duration: 400 }}
                >
                    <div class="bckg"></div>
                    <div class="card-value">
                        {#if card.value === "Skip"}
                            <img src="/images/skip.png" alt="Skip" class="card-icon" />
                        {:else if card.value === "Rev"}
                            <img src="/images/reverse.png" alt="Reverse" class="card-icon" />
                        {:else if card.value === "Wild"}
                            <img src="/images/wild.png" alt="Wild" class="card-icon" />
                        {:else}
                            <span class="text-value" class:small-text={card.value.length > 1}>{card.value}</span>
                        {/if}
                    </div>
                </div>
            {/each}
        </div>
    </div>

    <div id="player_left" style="position: relative; width: 100%; height: 100%;">
        <div
            style="position: absolute; top: 0px; width: 100%; text-align: center; font-weight: bold; color: white;"
        >
            {leftPlayer ? leftPlayer.username : "Waiting..."}
        </div>
        <div
            class="player_hand"
            style="
            position: absolute; top: 45%; left: 50%;
            transform: translate(-50%, -50%) rotate(90deg);
            width: calc({leftPlayer?.card_count ?? 0} * 2.2em + 7.2em);
            height: calc(var(--cardSize) * 1.5357);
        "
        >
            {#each Array(leftPlayer?.card_count ?? 0) as _, n (n)}
                <div class="card turned" data-index={n} in:fly={{ y: -200, duration: 400 }}>
                    <div class="bckg"></div>
                </div>
            {/each}
        </div>
    </div>

    <div id="player_top" style="position: relative; width: 100%; height: 100%;">
        <div
            style="position: absolute; top: 0px; width: 100%; text-align: center; font-weight: bold; color: white;"
        >
            {topPlayer ? topPlayer.username : "Waiting..."}
        </div>
        <div
            class="player_hand"
            style="
            position: absolute; top: 50%; left: 50%;
            transform: translate(-50%, -50%) scaleY(-1);
            width: calc({topPlayer?.card_count ?? 0} * 2.2em + 7.2em);
            height: calc(var(--cardSize) * 1.5357);
        "
        >
            {#each Array(topPlayer?.card_count ?? 0) as _, n (n)}
                <div class="card turned" data-index={n} in:fly={{ y: -200, duration: 400 }}>
                    <div class="bckg"></div>
                </div>
            {/each}
        </div>
    </div>

    <div id="player_right" style="position: relative; width: 100%; height: 100%;">
        <div
            style="position: absolute; top: 0px; width: 100%; text-align: center; font-weight: bold; color: white;"
        >
            {rightPlayer ? rightPlayer.username : "Waiting..."}
        </div>
        <div
            class="player_hand"
            style="
            position: absolute; top: 45%; left: 50%;
            transform: translate(-50%, -50%) rotate(-90deg);
            width: calc({rightPlayer?.card_count ?? 0} * 2.2em + 7.2em);
            height: calc(var(--cardSize) * 1.5357);
        "
        >
            {#each Array(rightPlayer?.card_count ?? 0) as _, n (n)}
                <div class="card turned" data-index={n} in:fly={{ y: -200, duration: 400 }}>
                    <div class="bckg"></div>
                </div>
            {/each}
        </div>
    </div>

    <div id="piles_area">
        <div id="draw_pile" onclick={handleDrawClick} role="button" tabindex="0">
            <div class="card turned top-card">
                <div class="bckg"></div>
            </div>
            <div class="card turned pile"><div class="bckg"></div></div>
        </div>

        <div id="discard_pile">
            {#if storeGame.state?.top_card}
                <div class="card top-card {storeGame.state.top_card.color}">
                    <div class="bckg"></div>
                    <div class="card-value" style="z-index: 10;">
                        {#if storeGame.state.top_card.value === "Skip"}
                            <img src="/images/skip.png" alt="Skip" class="card-icon" />
                        {:else if storeGame.state.top_card.value === "Rev"}
                            <img src="/images/reverse.png" alt="Reverse" class="card-icon" />
                        {:else if storeGame.state.top_card.value === "Wild"}
                            <img src="/images/wild.png" alt="Wild" class="card-icon" />
                        {:else}
                            <span class="text-value" class:small-text={storeGame.state.top_card.value.length > 1}>
                                {storeGame.state.top_card.value}
                            </span>
                        {/if}
                    </div>
                </div>
            {:else}
                <div class="card top-card {gameColor}">
                    <div class="bckg"></div>
                </div>
            {/if}
            <div class="card pile"><div class="bckg"></div></div>
        </div>
    </div>
</div>

<style>
    .sfondo-gioco {
        position: fixed;
        top: 0;
        left: 0;
        width: 100vw;
        height: 100vh;
        background-image: url('/images/background_red_dark.png');
        background-size: cover;
        background-position: center;
        background-repeat: no-repeat;
        z-index: 1;
        pointer-events: none;
    }

    :global(body) {
        margin: 0;
        padding: 0;
        overflow: hidden;
        background-color: transparent;
    }
    
    .card-value {
        position: absolute;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        font-weight: bold;
        pointer-events: none;
        display: flex;
        justify-content: center;
        align-items: center;
        width: 100%;
        height: 100%;
    }

    .card-icon {
        width: 50%;
        height: 50%;
        object-fit: contain;
    }

    .text-value {
        font-size: 2em;
    }

    .text-value.small-text {
        font-size: 1em;
    }

    .game-field {
        position: relative;
        z-index: 2;
        height: 100vh;
        display: grid;
        justify-content: center;
        align-content: center;
        grid-gap: 0.5em;
        grid-template-columns: var(--playerSpace) var(--fieldSize) var(--playerSpace);
        grid-template-rows: var(--playerSpace) var(--fieldSize) var(--playerSpace);
    }

    .game-field.perspective {
        transform: perspective(1200px) rotateX(10deg);
    }

    .game-field #piles_area {
        position: relative;
        border-radius: 4em;
        transition: 200ms;
    }

    .game-field.yellow #piles_area { background-color: rgba(252, 246, 4, 0.3); }
    .game-field.blue #piles_area { background-color: rgba(4, 147, 222, 0.3); }
    .game-field.red #piles_area { background-color: rgba(220, 37, 28, 0.3); }
    .game-field.green #piles_area { background-color: rgba(1, 141, 65, 0.3); }

    /* --- MODIFICHE DI POSIZIONAMENTO --- */

    #piles_area { 
        grid-area: 2 / 2; 
        /* Sposta l'intero quadrato overlay centrale verso l'alto */
        transform: translateY(-2.8em); 
    }
	
	#player_top { 
        grid-area: 2 / 2; 
        transform: translateY(-2.8em); 
    }

    #player_left { 
        grid-area: 2 / 1; 
        /* Spinge il giocatore di sinistra verso l'esterno dello schermo */
        transform: translateX(-4.5em); 
    }

    #player_right { 
        grid-area: 2 / 3; 
        /* Spinge il giocatore di destra verso l'esterno dello schermo */
        transform: translateX(4.5em); 
    }

    #player { grid-area: 3 / 2; }
    #player_top { grid-area: 1 / 2; }

    /* ----------------------------------- */

    #draw_pile {
        position: absolute;
        left: 5em;
        top: 7em;
    }
    #draw_pile .card.top-card,
    #draw_pile .card.pile {
        position: absolute;
    }
    #draw_pile .card.pile {
        box-shadow:
            0px 2px white,
            0px 4px var(--shadowColor),
            0px 6px white,
            0px 8px var(--shadowColor),
            0px 10px white,
            0px 12px var(--shadowColor),
            0px 14px white,
            0px 16px var(--shadowColor),
            0px 18px white,
            0px 20px var(--shadowColor);
    }
    #draw_pile .card.pile:hover { transform: none; }
    #draw_pile .card.top-card { z-index: 100; box-shadow: none; }
    #draw_pile .card.top-card:hover {
        box-shadow: 0px 4px var(--shadowColor);
        cursor: pointer;
        transform: translateY(1em);
    }

    #discard_pile {
        position: absolute;
        left: 12em;
        top: 7.4em;
    }
    #discard_pile .card.top-card,
    #discard_pile .card.pile {
        position: absolute;
    }
    #discard_pile .card.pile {
        box-shadow:
            0px 2px white,
            0px 4px var(--shadowColor),
            0px 6px white,
            0px 8px var(--shadowColor);
    }
    #discard_pile .card.pile:hover { transform: none; }
    #discard_pile .card.top-card { z-index: 100; box-shadow: none; }

    .player_hand { position: relative; }
    .player_hand .card { position: absolute; }

    .player_hand .card:nth-child(1) { left: calc(1 * 2.2em); }
    .player_hand .card:nth-child(2) { left: calc(2 * 2.2em); }
    .player_hand .card:nth-child(3) { left: calc(3 * 2.2em); }
    .player_hand .card:nth-child(4) { left: calc(4 * 2.2em); }
    .player_hand .card:nth-child(5) { left: calc(5 * 2.2em); }
    .player_hand .card:nth-child(6) { left: calc(6 * 2.2em); }
    .player_hand .card:nth-child(7) { left: calc(7 * 2.2em); }
    .player_hand .card:nth-child(8) { left: calc(8 * 2.2em); }
    .player_hand .card:nth-child(9) { left: calc(9 * 2.2em); }
    .player_hand .card:nth-child(10) { left: calc(10 * 2.2em); }
    .player_hand .card:nth-child(11) { left: calc(11 * 2.2em); }
    .player_hand .card:nth-child(12) { left: calc(12 * 2.2em); }
    .player_hand .card:nth-child(13) { left: calc(13 * 2.2em); }
    .player_hand .card:nth-child(14) { left: calc(14 * 2.2em); }
    .player_hand .card:nth-child(15) { left: calc(15 * 2.2em); }

    #player .player_hand .card { cursor: pointer; }
    #player .player_hand .card:hover {
        transform-origin: left bottom;
        transform: rotate(-10deg) translateY(-0.5em);
    }
    #player .player_hand .card:hover ~ .card { transform: translateX(2em); }

    :root {
        --cardSize: 5em;
        --redCard: #dc251c;
        --yellowCard: #fcf604;
        --blueCard: #0493de;
        --greenCard: #018d41;
        --blackCard: #1f1b18;
        --lowShadow: 0 1px 3px rgba(0, 0, 0, 0.12), 0 1px 2px rgba(0, 0, 0, 0.24);
        --lowShadowHover: 0 3px 6px rgba(0, 0, 0, 0.16), 0 3px 6px rgba(0, 0, 0, 0.23);
        --shadowColor: rgba(0, 0, 0, 0.16);
        --fieldSize: 24em;
        --playerSpace: 12em;
    }

    .card {
        display: inline-block;
        background-color: white;
        border: 1px solid #ccc;
        border-radius: 0.8em;
        padding: 0.3em;
        box-shadow: var(--lowShadow);
        transition: 200ms;
        position: relative;
    }

    .card .bckg {
        width: var(--cardSize);
        height: calc(var(--cardSize) * 1.5357);
        border-radius: 0.5em;
        overflow: hidden;
        position: relative;
    }

    .card .bckg::before {
        content: "";
        width: var(--cardSize);
        height: calc(var(--cardSize) * 1.3);
        background-color: white;
        position: absolute;
        left: 50%;
        top: 50%;
        transform: translate(-50%, -50%) rotate(10deg);
        transform-origin: center center;
        border-radius: 90% 40%;
    }

    .card.red { color: var(--redCard); }
    .card.red .bckg { background-color: var(--redCard); }
    .card.yellow { color: var(--yellowCard); }
    .card.yellow .bckg { background-color: var(--yellowCard); }
    .card.blue { color: var(--blueCard); }
    .card.blue .bckg { background-color: var(--blueCard); }
    .card.green { color: var(--greenCard); }
    .card.green .bckg { background-color: var(--greenCard); }
    .card.black { color: var(--blackCard); }
    .card.black .bckg { background-color: var(--blackCard); }

    .card.turned:hover { cursor: default; }
    .card.turned .bckg { background-color: var(--blackCard); }
    .card.turned .bckg::before { background-color: var(--redCard); }
</style>