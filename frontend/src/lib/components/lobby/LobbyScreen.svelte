<script lang="ts">
    import { storeLobby } from "../../stores/lobby.svelte";
    import { storeAuth } from "../../stores/auth.svelte";

    let isHost = $derived(storeAuth.username === storeLobby.current?.host);
</script>

<link rel="preload" href="fonts/JetBrainsMono.woff2" as="font" type="font/woff2" />

<div>
    <button class="join-button" onclick={storeLobby.leave}>Leave</button>

    <button
        class="join-button"
        onclick={storeLobby.startGame}
        disabled={!isHost || storeLobby.current!.members.length < 2}>
        Start Game
    </button>

    <h1>{`${storeLobby.current!.name} | ${storeLobby.current!.invite_code}`}</h1>
    
    <ul class="memberlist">
        {#each storeLobby.current!.members as member}
            <li class="member">
                <span class="member-name">{member.username}</span>
                
                {#if member.is_host}
                    <span style="color: lightgoldenrodyellow"> 󱟜 </span>
                {/if}
                
                {#if isHost && !member.is_host}
                    <button 
                        class="transfer-button" >
                        <!-- onclick={() => storeLobby.transferHost(member.username)} bho è stato autofatto-->
                        <span style="color: lightgoldenrodyellow"> 󱟜 </span>
                    </button>
                {/if}

                <span class:off={!member.is_connected} class:on={member.is_connected}>
                    {member.is_connected ? "\uf14a" : "\uf2d3"}
                </span>
            </li>
        {/each}
    </ul>
</div>

<style>
    @font-face {
        font-family: "JetBrainsMono";
        src: url("/fonts/JetBrainsMono.woff2") format("woff2");
        font-weight: 400;
        font-style: normal;
        font-display: swap;
    }

    .join-button {
        padding: 8px 16px;
        background: var(--accent);
        color: white;
        border: none;
        border-radius: 6px;
        font-size: 14px;
        font-weight: 500;
        cursor: pointer;
        transition:
            background 0.2s,
            opacity 0.2s;
    }

    .join-button:disabled {
        background: #444444;
        color: #888888;
        cursor: not-allowed;
        opacity: 0.6;
    }

    .memberlist {
        list-style-type: none;
        padding: 0;
    }

    .member {
        font-family: "JetBrainsMono";
        color: lightgray;
        display: flex;
        align-items: center;
        gap: 10px;
        margin-bottom: 8px;
    }

    .member-name {
        min-width: 120px; /* Allinea gli elementi graficamente */
    }

    .transfer-button {
        padding: 4px 8px;
        background: #333333;
        color: #cccccc;
        border: 1px solid #555555;
        border-radius: 4px;
        font-size: 11px;
        cursor: pointer;
        font-family: "JetBrainsMono";
        transition: background 0.2s, color 0.2s;
        display: flex;
        align-items: center;
        justify-content: center;
    }

    .transfer-button:hover {
        background: rgba(250, 250, 210, 0.39);
        color: #111111;
        border-color: rgba(250, 250, 210, 0.322);
    }

    .off {
        color: salmon;
    }

    .on {
        color: lightgreen;
    }
</style>