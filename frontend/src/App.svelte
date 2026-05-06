<script lang="ts">
    import SocketManager from "./lib/SocketManager.svelte";
    import svelteLogo from "./assets/svelte.svg";
    import viteLogo from "./assets/vite.svg";
    import heroImg from "./assets/hero.png";
    import Counter from "./lib/Counter.svelte";

    let currentScreen: "index" | "buttonTest" = "index";
    let syncedData: { clicks: number; lastClicker: string };

    async function handleSubmit(event: SubmitEvent) {
        const formData = new FormData(event.target as HTMLFormElement);

        try {
            const response = await fetch(
                `${window.location.protocol}//${window.location.host}/room`,
                {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ topic: formData.get("topic") }),
                },
            );

            const result = await response.json();

            if (result.status === "OK") {
                let socketManager = SocketManager.getInstance();
                await socketManager.connect();
                const myData: {
                    action: "sync_data";
                    room: string;
                    username: string;
                } = await socketManager.emitAndWait(
                    "join",
                    { topic: result.topic },
                    "sync_data",
                );

                if (myData.room) {
                    const queriedData = await socketManager.emitAndWait(
                        "query",
                        {},
                        "queried",
                    );

                    syncedData = {
                        clicks: queriedData.clicks,
                        lastClicker: queriedData.lastClicker,
                    };

                    currentScreen = "buttonTest";
                }
            }
        } catch (error) {
            console.error("Errore durante il fetch:", error);
        }
    }
</script>

<div>
    <section id="center">
        <div class="hero">
            <img src={heroImg} class="base" width="170" height="179" alt="" />
            <img src={svelteLogo} class="framework" alt="Svelte logo" />
            <img src={viteLogo} class="vite" alt="Vite logo" />
        </div>
        <div>
            <h1>Test Uni-Uno</h1>
            <p>
                Edit <code>src/App.svelte</code> and save to test
                <code>HMR</code>
            </p>
            <br />
        </div>
    </section>

    <div class="ticks"></div>

    {#if currentScreen === "index"}
        <section id="next-steps">
            <div id="docs">
                <h1>E' un test</h1>
                <h2>non si vede?</h2>
            </div>
            <div>
                <h1>Form per Accedere</h1>
                <form on:submit|preventDefault={handleSubmit}>
                    <label>
                        Nome Stanza:
                        <input class="counter" name="topic" id="topic" />
                    </label>
                    <button class="counter" type="submit">Accedi!</button>
                </form>
            </div>
        </section>
        <div class="ticks"></div>
    {:else if currentScreen === "buttonTest"}
        <section id="next-steps">
            <div id="docs">
                <h1>{SocketManager.getInstance().syncedSocketData.room}</h1>
                <h2>E' la stanza corrente.</h2>
            </div>
            <div>
                <h1>Clicca!</h1>
                <h2>Aumenterà a tutti 😉</h2>
                <Counter data={syncedData}></Counter>
            </div>
        </section>
        <div class="ticks"></div>
    {/if}

    <div class="ticks"></div>
    <section id="spacer"></section>
</div>
