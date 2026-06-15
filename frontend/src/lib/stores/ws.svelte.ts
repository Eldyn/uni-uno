/**
 * @file ws.svelte.ts
 * @brief Communication utility and wrapper for the WebSocket protocol.
 * Handles the connection, message routing (handlers) and implements a
 * synchronous/asynchronous pattern (emit/wait) using Svelte 5 reactive state.
 */

/**
 * @brief Dictionary of constants for the actions sent from the Client to the Server.
 */
export const ClientAction = {
    GamePlayCard: "game_play_card",
    GameDrawCard: "game_draw_card",
    GameSubmitInput: "game_submit_input",
    GameCallUno: "game_call_uno",
    GameExit: "game_exit",

    LobbyListSavedMatches: "lobby_list_saved_matches",
    LobbyDeleteSavedMatch: "lobby_delete_saved_match",
    LobbyResumeSavedMatch: "lobby_resume_saved_match",

    LobbyList: "lobby_list",
    LobbyCreate: "lobby_create",
    LobbyJoin: "lobby_join",
    LobbyRejoin: "lobby_rejoin",
    LobbyLeave: "lobby_leave",
    LobbyPromote: "lobby_promote",
    LobbyKick: "lobby_kick",
    LobbyUpdateSettings: "lobby_update_settings",
    LobbyStartMatch: "lobby_start_match",

    ChatSend: "chat_send"
} as const;

/**
 * @brief Dictionary of constants for the actions sent from the Server to the Client.
 */
export const ServerAction = {
    Success: "success",
    Error: "error",

    GameStateUpdated: "game_state_updated",
    GameOver: "game_over",

    LobbyEvicted: "lobby_evicted",
    LobbyList: "lobby_list",
    LobbyUpdated: "lobby_updated",
    LobbyJoined: "lobby_joined",
    LobbyLeft: "lobby_left",

    ChatMessage: "chat_message"
} as const;

export type ClientActionType = (typeof ClientAction)[keyof typeof ClientAction];
export type ServerActionType = (typeof ServerAction)[keyof typeof ServerAction];

export type ServerActionDef = ServerActionType | string;

/**
 * @typedef MessageHandler
 * @brief Signature of the callback function for intercepting WebSocket messages.
 */
export type MessageHandler = (data: Record<string, unknown>) => void;

/**
 * @interface ConnectionStatus
 * @brief Represents the reactive state of the WebSocket connection, useful for the UI.
 */
export interface ConnectionStatus {
    /** Current state of the connection. */
    status: "disconnected" | "connecting" | "connected";
    /** Username synchronized from the server. */
    username: string;
    /** Name of the current room. */
    room: string;
    /** Invite code of the current lobby. */
    lobby_code: string;
}

/**
 * @class WsResponse
 * @brief Unified wrapper for WebSocket message responses.
 * Mimics the HTTP Fetch API to provide a linear and predictable control flow
 * when awaiting responses from the server.
 * @tag FRONT-WS-001
 */
export class WsResponse {
    /** True if the action returned by the server is NOT an error. */
    public readonly ok: boolean;
    /** The ServerAction command returned in the packet. */
    public readonly action: string;
    /** The unique ID of the original request forwarded by the client, if present. */
    public readonly request_id?: string;
    /** The raw JSON dictionary with the data returned by the server. */
    public readonly data: Record<string, unknown>;

    constructor(rawData: Record<string, unknown>) {
        this.data = rawData;
        this.action = (rawData.action as string) || "";
        this.request_id = rawData.request_id as string;

        this.ok = this.action !== "error";
    }

    /**
     * @brief Safely extracts the error reason returned by the server.
     * @returns The error message or a default fallback.
     */
    get reason(): string {
        return (this.data.reason as string) || "Unknown Server Error";
    }

    /**
     * @brief Extracts a typed value from the payload.
     * @param key The key to look up in the JSON dictionary.
     * @returns The typed value or null if the key does not exist.
     */
    get<T>(key: string): T | null {
        return key in this.data ? (this.data[key] as T) : null;
    }

    /**
     * @brief Extracts a typed value from the payload, providing a fallback when absent.
     * @param key The key to look up in the JSON payload.
     * @param fallback The safe value to return.
     */
    getOr<T>(key: string, fallback: T): T {
        return key in this.data ? (this.data[key] as T) : fallback;
    }
}

/**
 * @interface PendingRequest
 * @brief Internal interface used to track requests awaiting a response (emitAndWait).
 */
interface PendingRequest {
    resolve: (res: WsResponse) => void;
    reject: (err: Error) => void;
    timer: ReturnType<typeof setTimeout>;
}

/**
 * @class WebSocketClient
 * @brief Stateful and reactive WebSocket client.
 * Handles automatic reconnections (Exponential Backoff), correlation between requests
 * and responses, and encapsulates the connection states using Svelte 5 (`$state`).
 * @tag FRONT-WS-002
 */
export class WebSocketClient {
    /** Native WebSocket instance (reactive). */
    socket = $state<WebSocket | null>(null);

    /** Connection metadata and state (reactive to interface with the UI). */
    connectionStatus = $state<ConnectionStatus>({
        status: "disconnected",
        username: "",
        room: "",
        lobby_code: ""
    });

    private _nextRequestId = 1;
    private pendingRequests = new Map<string, PendingRequest>();

    private onHandlers = new Map<string, Set<MessageHandler>>();
    private openHandlers: Array<() => void | Promise<void>> = [];
    private closeHandlers: Array<() => void | Promise<void>> = [];

    private visibilityListenerAttached = false;

    private reconnectTimer: ReturnType<typeof setTimeout> | null = null;
    private reconnectDelayMs = 1000;
    private intentionalClose = false;

    /**
     * @brief Checks whether the WebSocket is currently open and active.
     */
    get isConnected(): boolean {
        return this.socket?.readyState === WebSocket.OPEN;
    }

    /**
     * @brief Installs and establishes the WebSocket connection. Safe to call multiple times.
     * @returns Promise resolved when the connection opens successfully.
     */
    async connect(): Promise<void> {
        if (this.isConnected) return;

        this.intentionalClose = false;
        this.#attachVisibilityListener();
        await this._connectOnce();
    }

    /**
     * @brief Intentionally disconnects the WebSocket, blocking the auto-reconnection system.
     * @param code The WebSocket close code.
     * @param reason The textual reason for the closure.
     */
    disconnect(code: number, reason: string): void {
        this.intentionalClose = true;

        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }

        this.socket?.close(code, reason);
        this.socket = null;
        this.connectionStatus.status = "disconnected";

        for (const handler of this.closeHandlers) {
            try {
                handler();
            } catch (error) {
                console.error("onClose Handler failed:", error);
            }
        }
    }

    /**
     * @brief Sends an asynchronous message to the server without awaiting any response (Fire and Forget).
     * Does not attach any `request_id` to the packet.
     * @param action The action to perform (ClientAction).
     * @param payload Optional data to attach in JSON format.
     */
    emit(action: string, payload: Record<string, unknown> = {}): void {
        if (this.isConnected) {
            this.socket!.send(JSON.stringify({ action, ...payload }));
        }
    }

    /**
     * @brief Sends a message and awaits the correlated response from the server by generating a `request_id`.
     * Resolves to a `WsResponse` even for logical errors generated by the server, but rejects
     * on infrastructure failures (e.g. network drop or Timeout).
     * @param action The action to perform (ClientAction).
     * @param payload Optional data to attach.
     * @param timeoutMs Maximum wait time before aborting (Default: 5000ms).
     * @returns Promise resolved with the server's response packet.
     */
    emitAndWait(
        action: string,
        payload: Record<string, unknown> = {},
        timeoutMs: number = 5000
    ): Promise<WsResponse> {
        return new Promise((resolve, reject) => {
            const requestId = String(this._nextRequestId++);

            const timer = setTimeout(() => {
                this.pendingRequests.delete(requestId);
                reject(new Error(`Timeout: no response for "${action}" (request_id=${requestId})`));
            }, timeoutMs);

            this.pendingRequests.set(requestId, { resolve, reject, timer });

            if (this.isConnected) {
                this.socket!.send(JSON.stringify({ action, request_id: requestId, ...payload }));
            } else {
                this.pendingRequests.delete(requestId);
                clearTimeout(timer);
                reject(new Error("WebSocket not connected"));
            }
        });
    }

    /**
     * @brief Registers a global listener to intercept specific Server Actions.
     * @param action The server action to listen for (use "*" to listen to everything).
     * @param handler The callback function triggered upon receiving the message.
     * @returns A lambda to remove the event subscription.
     */
    on(action: ServerActionDef | "*", handler: MessageHandler): () => void {
        if (!this.onHandlers.has(action)) this.onHandlers.set(action, new Set());
        this.onHandlers.get(action)!.add(handler);

        return () => this.onHandlers.get(action)?.delete(handler);
    }

    /**
     * @brief Registers a listener triggered immediately when the socket opens.
     * @returns Function to remove the hook.
     */
    onOpen(handler: () => void | Promise<void>): () => void {
        this.openHandlers.push(handler);

        // Returns the cancellation function (unsubscriber)
        return () => {
            this.openHandlers = this.openHandlers.filter((h) => h !== handler);
        };
    }

    /**
     * @brief Registers a listener triggered when the socket closes.
     * @returns Function to remove the hook.
     */
    onClose(handler: () => void | Promise<void>): () => void {
        this.closeHandlers.push(handler);

        // Returns the cancellation function (unsubscriber)
        return () => {
            this.closeHandlers = this.closeHandlers.filter((h) => h !== handler);
        };
    }

    /**
     * @brief Internal routine to establish a single raw connection and bind the native events.
     */
    private async _connectOnce(): Promise<void> {
        return new Promise(async (resolve, reject) => {
            const protocol = location.protocol === "https:" ? "wss:" : "ws:";
            const wsInstance = new WebSocket(`${protocol}//${location.host}`);
            this.connectionStatus.status = "connecting";

            wsInstance.onopen = async () => {
                this.socket = wsInstance;
                this.connectionStatus.status = "connected";
                this.reconnectDelayMs = 1000;

                for (const handler of this.openHandlers) {
                    try {
                        handler();
                    } catch (error) {
                        console.error("onOpen Handler failed:", error);
                    }
                }

                resolve();
            };

            wsInstance.onerror = (e) => reject(e);

            wsInstance.onclose = () => {
                if (this.socket === wsInstance) {
                    this.socket = null;
                    this.connectionStatus.status = "disconnected";
                }
                this._scheduleReconnect();
            };

            wsInstance.onmessage = (e: MessageEvent) => {
                try {
                    const data = JSON.parse(e.data as string) as Record<string, unknown>;
                    if (data.action === "sync_data") {
                        this.connectionStatus.username = data.username as string;
                    }
                    this._dispatch(data);
                } catch (err) {
                    console.error(err);
                }
            };
        });
    }

    /**
     * @brief Routes the incoming message, resolving any pending requests or triggering global listeners.
     */
    private _dispatch(data: Record<string, unknown>) {
        const action = data.action as string;
        const requestId = data.request_id as string;

        // INFO: Resolve the pending promises if there is a match with the ID
        if (requestId && this.pendingRequests.has(requestId)) {
            const pending = this.pendingRequests.get(requestId)!;
            this.pendingRequests.delete(requestId);
            clearTimeout(pending.timer);

            // INFO: Wrap the payload in our HTTP-like envelope and resolve
            const response = new WsResponse(data);
            pending.resolve(response);
        }

        // INFO: Trigger the standard global listeners
        if (action) {
            this.onHandlers.get(action)?.forEach((handler) => handler(data));
            this.onHandlers.get("*")?.forEach((handler) => handler(data));
        }
    }

    /**
     * @brief Implements an Exponential Backoff algorithm to attempt reconnections.
     * Doubles the delay on every failed attempt up to a maximum cap.
     */
    private _scheduleReconnect() {
        if (this.intentionalClose) return;
        if (this.reconnectTimer) clearTimeout(this.reconnectTimer);

        this.reconnectTimer = setTimeout(() => {
            this.reconnectTimer = null;

            if (document.visibilityState === "hidden") return;

            this.reconnectDelayMs = Math.min(this.reconnectDelayMs * 2, 16_000);

            this._connectOnce().catch(() => {
                this._scheduleReconnect();
            });
        }, this.reconnectDelayMs);
    }

    /**
     * @brief Registers a listener on the document to detect the user returning to the tab.
     * Disables the backoff timer, forcing an immediate reconnection when the tab becomes visible again.
     */
    #attachVisibilityListener() {
        if (this.visibilityListenerAttached) return;
        this.visibilityListenerAttached = true;

        document.addEventListener("visibilitychange", () => {
            if (document.visibilityState === "visible" && !this.isConnected && !this.intentionalClose) {
                // Cancel the pending timers and force a connection
                if (this.reconnectTimer) {
                    clearTimeout(this.reconnectTimer);
                    this.reconnectTimer = null;
                }

                this.reconnectDelayMs = 1000; // Reset the backoff
                this._connectOnce().catch(() => this._scheduleReconnect());
            }
        });
    }
}

// Export a global singleton instance for use throughout the Svelte application
export const ws = new WebSocketClient();
