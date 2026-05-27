/**
 * This store owns:
 *   - The current lobby the user is in
 *   - The public list of browseable lobbies
 *   - The reconnect-to-lobby flow that previously lived inside ws
 */

import { storeNavigation } from "./navigation.svelte";
import { storeToast } from "./toast.svelte";
import { ClientAction, ServerAction, ws } from "./ws.svelte";

export interface LobbyMember {
    username: string;
    is_connected: boolean;
    is_host: boolean;
}

export interface Lobby {
    invite_code: string;
    host: string;
    name: string;
    member_count: number;
    members: LobbyMember[];
}

class StoreLobby {
    // ── Reactive state ──────────────────────────────────────────────────────
    current = $state<Lobby | null>(null);
    available = $state<Lobby[]>([]);
    isLoadingList = $state(false);
    isLoadingJoin = $state(false);

    get isInLobby(): boolean {
        return this.current !== null;
    }

    /**
     * Wire up all WS listeners and attempt to restore a previous lobby session.
     */
    constructor() {
        ws.onOpen(async () => {
            this.#registerListeners();
            await this.#tryRejoin();
        });
    }

    async create(data: { is_public: boolean; name: string }): Promise<void> {
        this.isLoadingJoin = true;

        await ws.connect();

        const response = await ws.emitAndWait(ClientAction.LobbyCreate, { ...data });

        if (!response.ok) {
            storeToast.error(response.reason);
        }

        this.isLoadingJoin = false;
    }

    async join(code: string): Promise<void> {
        if (!code) {
            storeToast.error("Enter an invite code.");
            return;
        }

        this.isLoadingJoin = true;

        await ws.connect();

        const response = await ws.emitAndWait(ClientAction.LobbyJoin, { code });

        if (!response.ok) {
            storeToast.error(response.reason);
        }

        this.isLoadingJoin = false;
    }

    async fetchList(): Promise<void> {
        this.isLoadingList = true;

        await ws.connect();

        const response = await ws.emitAndWait(ClientAction.LobbyList);

        if (response.ok) {
            this.available = response.get<Lobby[]>("lobbies") ?? [];
        }

        this.isLoadingList = false;
    }

    leave(): void {
        ws.emit("lobby_leave");
    }

    /**
     * Register all server-to-client event handlers for lobby events.
     * These are permanent for the lifetime of the app — no cleanup needed.
     */
    #registerListeners(): void {
        ws.on(ServerAction.LobbyJoined, (data) => {
            this.current = data.lobby as Lobby;
            localStorage.setItem("lobby_code", (data.lobby as Lobby).invite_code);
            storeNavigation.goto("lobby");
            storeToast.success("Joined lobby successfully!");
        });

        ws.on(ServerAction.LobbyUpdated, (data) => {
            if (this.current) {
                this.current = data.lobby as Lobby;
            }
            // Also update the entry in the available list if it's there
            const code = (data.lobby as Lobby)?.invite_code;
            if (code) {
                const idx = this.available.findIndex((l) => l.invite_code === code);
                if (idx !== -1) this.available[idx] = data.lobby as Lobby;
            }
        });

        ws.on(ServerAction.LobbyLeft, () => {
            this.#reset();
            storeNavigation.goto("lobbies");
        });

        ws.on(ServerAction.LobbyEvicted, () => {
            this.#reset();
            storeNavigation.goto("lobbies");
        });
    }

    /**
     * On every connection (including reconnects), check localStorage for a
     * saved lobby code and attempt to silently rejoin.
     *
     * Previously this lived in ws._connectOnce — it belongs here because
     * it requires knowledge of lobby domain concepts.
     */
    async #tryRejoin(): Promise<void> {
        const code = localStorage.getItem("lobby_code");
        if (!code) return;

        const response = await ws.emitAndWait(ClientAction.LobbyRejoin, { code }).catch(() => null);

        if (!response || !response.ok) {
            localStorage.removeItem("lobby_code");
            if (response) storeToast.error(`Could not rejoin lobby: ${response.reason}`);
            return;
        }

        if (response.action === ServerAction.LobbyEvicted) {
            localStorage.removeItem("lobby_code");
            return;
        }

        const lobby = response.get<Lobby>("lobby");
        if (!lobby) {
            storeToast.error(`Could not rejoin lobby: Unknown Error`);
            localStorage.removeItem("lobby_code");
            return;
        }

        this.current = lobby;
        storeNavigation.goto("lobby");
    }

    #reset(): void {
        this.current = null;
        localStorage.removeItem("lobby_code");
    }
}

export const storeLobby = new StoreLobby();
