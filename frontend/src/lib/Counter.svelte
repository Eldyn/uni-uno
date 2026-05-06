<script lang="ts">
    import SocketManager from "./SocketManager.svelte";
    //@ts-ignore
    let { data }: { clicks: number; lastClicks: string } = $props();

    let counter = $state(data.clicks);
    let lastClicker = $state(data.lastClicker);

    SocketManager.getInstance().socket.addEventListener("message", (event) => {
        if (event.data.startsWith("{")) {
            const data = JSON.parse(event.data);
            if (data.action === "sync_count") {
                counter = data.count;
                lastClicker = data.last_clicker;
            }
        }
    });

    async function buttonClick() {
        let data = await SocketManager.getInstance().emitAndWait(
            "click",
            {},
            "sync_count",
        );

        counter = Number(data.count);
        lastClicker = String(data.last_clicker);
    }
</script>

<button type="button" class="counter" onclick={buttonClick}
    >Count is {counter}</button
>
<p>L'ultimo a cliccare è stato: {lastClicker}</p>
