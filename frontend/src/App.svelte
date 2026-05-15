<script lang="ts">
    import { connect, emitAndWait, socketState } from "./lib/socket.svelte";
    import Counter from "./lib/Counter.svelte";

    import svelteLogo from "./assets/svelte.svg";
    import viteLogo from "./assets/vite.svg";
    import heroImg from "./assets/hero.png";

    // Gestione delle pagine ! Temporaneo
    let currentScreen: "auth" | "index" | "game" = "auth";
    let syncedData = { clicks: 0, lastClicker: "Nessuno" };

    // Stato del colore in game
    let gameColor = "blue";

    // REGISTRAZIONE
    async function handleRegister(event: SubmitEvent) {
        const formData = new FormData(event.target as HTMLFormElement);
        try {
            const response = await fetch(
                `${window.location.origin}/auth/register`,
                {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({
                        username: formData.get("username"),
                        email: formData.get("email"),
                        password: formData.get("password"),
                    }),
                },
            );

            if (response.ok) {
                alert("Registrazione completata! Ora effettua il login.");
            } else {
                console.error("Errore nella registrazione");
            }
        } catch (error) {
            console.error("Fetch error:", error);
        }
    }

    // LOGIN
    async function handleLogin(event: SubmitEvent) {
        const formData = new FormData(event.target as HTMLFormElement);
        try {
            const response = await fetch(
                `${window.location.origin}/auth/login`,
                {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({
                        email: formData.get("email"),
                        password: formData.get("password"),
                    }),
                },
            );

            if (response.ok) {
                currentScreen = "index";
            } else {
                alert("Credenziali errate");
            }
        } catch (error) {
            console.error("Fetch error:", error);
        }
    }

    // ACCESSO ALLA STANZA
    async function handleSubmit(event: SubmitEvent) {
        const formData = new FormData(event.target as HTMLFormElement);
        try {
            const response = await fetch(`${window.location.origin}/room`, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ topic: formData.get("topic") }),
            });

            if (response.status === 422) {
                throw new Error(
                    "Input used illegal characters (or was too long)",
                );
            }

            const result = await response.json();

            if (result.status === "OK") {
                await connect("TODO_JWT_token");

                const myData = await emitAndWait(
                    "join",
                    { topic: result.topic },
                    "sync_data",
                );

                if (myData.room) {
                    const queriedData = await emitAndWait(
                        "query",
                        {},
                        "queried",
                    );

                    syncedData = {
                        clicks: queriedData.clicks as number,
                        lastClicker: queriedData.lastClicker as string,
                    };

                    currentScreen = "game";
                }
            }
        } catch (error) {
            console.error("Fetch error:", error);
        }
    }
</script>

<!-- Div per non vedere l'errore in main.js -->
<div id="app"></div>

<div>
    <section id="center">
        <div class="hero">
            <img src={heroImg} class="base" width="170" height="179" alt="" />
            <img src={svelteLogo} class="framework" alt="Svelte logo" />
            <img src={viteLogo} class="vite" alt="Vite logo" />
        </div>
    </section>

    <div class="ticks"></div>

    {#if currentScreen === "auth"}
        <section id="next-steps">
            <div id="docs">
                <h1>Form per Registrarsi</h1>
                <form on:submit|preventDefault={handleRegister}>
                    <label>
                        username:
                        <input class="counter" name="username" required />
                    </label>
                    <br />
                    <label>
                        email:
                        <input
                            class="counter"
                            name="email"
                            type="email"
                            required
                        />
                    </label>
                    <br />
                    <label>
                        password:
                        <input
                            class="counter"
                            name="password"
                            type="password"
                            required
                        />
                    </label>
                    <br />
                    <button class="counter" type="submit">Registrati!</button>
                </form>
            </div>

            <div>
                <h1>Form per Login</h1>
                <form on:submit|preventDefault={handleLogin}>
                    <label>
                        email:
                        <input
                            class="counter"
                            name="email"
                            type="email"
                            required
                        />
                    </label>
                    <br />
                    <label>
                        password:
                        <input
                            class="counter"
                            name="password"
                            type="password"
                            required
                        />
                    </label>
                    <br />
                    <button class="counter" type="submit">Login!</button>
                </form>
            </div>
        </section>
    {:else if currentScreen === "index"}
        <section id="next-steps">
            <div id="docs">
                <h1>Benvenuto!</h1>
                <h2>Inserisci il nome di una stanza per iniziare a giocare.</h2>
            </div>
            <div>
                <h1>Form per Accedere</h1>
                <form on:submit|preventDefault={handleSubmit}>
                    <label>
                        Nome Stanza:
                        <input
                            class="counter"
                            name="topic"
                            id="topic"
                            required
                        />
                    </label>
                    <button class="counter" type="submit"
                        >Accedi alla Stanza!</button
                    >
                </form>
            </div>
        </section>
    {:else if currentScreen === "game"}
        <section id="next-steps"></section>
        <div class="game-field perspective {gameColor}">
            <div id="player">
                (You)
                <div class="player_hand">
                    {#each Array(2) as _, r}
                        <div class="card red" data-key={r}>
                            <div class="bckg"></div>
                        </div>
                    {/each}

                    {#each Array(2) as _, b}
                        <div class="card blue" data-key={b}>
                            <div class="bckg"></div>
                        </div>
                    {/each}

                    <div class="card yellow"><div class="bckg"></div></div>
                    <div class="card green"><div class="bckg"></div></div>
                    <div class="card black"><div class="bckg"></div></div>
                </div>
            </div>

            <div id="player_left">
                Left Player
                <div class="player_hand">
                    {#each Array(7) as _, n}
                        <div class="card turned" data-index={n}>
                            <div class="bckg"></div>
                        </div>
                    {/each}
                </div>
            </div>

            <div id="player_top">
                Top Player
                <div class="player_hand">
                    {#each Array(7) as _, n}
                        <div class="card turned" data-index={n}>
                            <div class="bckg"></div>
                        </div>
                    {/each}
                </div>
            </div>

            <div id="player_right">
                Right Player
                <div class="player_hand">
                    {#each Array(7) as _, n}
                        <div class="card turned" data-index={n}>
                            <div class="bckg"></div>
                        </div>
                    {/each}
                </div>
            </div>

            <div id="piles_area">
                <div id="draw_pile">
                    <div class="card turned top-card">
                        <div class="bckg"></div>
                    </div>
                    <div class="card turned pile"><div class="bckg"></div></div>
                </div>
                <div id="discard_pile">
                    <div class="card top-card {gameColor}">
                        <div class="bckg"></div>
                    </div>
                    <div class="card pile"><div class="bckg"></div></div>
                </div>
            </div>
        </div>
    {/if}

    <div class="ticks"></div>
    <section id="spacer"></section>
    <div class="ticks"></div>
</div>
