/**
 * WebSocket communication utilities
 * Manages connection, message handlers, and emit/wait patterns
 */

export const ClientAction = {
    LobbyList: "lobby_list",
    LobbyCreate: "lobby_create",
    LobbyJoin: "lobby_join",
    LobbyLeave: "lobby_leave",
    LobbyInvite: "lobby_invite",
    LobbySetRuleset: "lobby_set_ruleset",
    LobbySetSkin: "lobby_set_skin",
    Ready: "ready",
    PlayCard: "play_card",
    DrawCard: "draw_card",
    CallUno: "call_uno",
    ChallengeUno: "challenge_uno",
    ChallengePlusFour: "challenge_plus_four",
    ChooseColor: "choose_color",
    AckGameOver: "ack_game_over",
    StorePurchase: "store_purchase",
    InventoryRequest: "inventory_request",
    ChatSend: "chat_send",
} as const;

export const ServerAction = {
    SyncData: "sync_data",
    Error: "error",
    LobbyList: "lobby_list",
    LobbyUpdated: "lobby_updated",
    LobbyJoined: "lobby_joined",
    LobbyLeft: "lobby_left",
    InviteCreated: "invite_created",
    PlayerJoined: "player_joined",
    PlayerLeft: "player_left",
    PlayerReady: "player_ready",
    GameStarting: "game_starting",
    RulesetSync: "ruleset_sync",
    StateUpdate: "state_update",
    CardRejected: "card_rejected",
    CardDrawn: "card_drawn",
    TurnChanged: "turn_changed",
    ColorChanged: "color_changed",
    UnoCallSuccess: "uno_call_success",
    UnoCallFailed: "uno_call_failed",
    UnoChallenged: "uno_challenged",
    CardPlayed: "card_played",
    PlayerDrewCard: "player_drew_card",
    SkipApplied: "skip_applied",
    ReverseApplied: "reverse_applied",
    DrawStackApplied: "draw_stack_applied",
    GameOver: "game_over",
    RoundOver: "round_over",
    InventorySync: "inventory_sync",
    PurchaseConfirmed: "purchase_confirmed",
    PurchaseFailed: "purchase_failed",
    ChatMessage: "chat_message",
} as const;

export type ClientActionType = (typeof ClientAction)[keyof typeof ClientAction];
export type ServerActionType = (typeof ServerAction)[keyof typeof ServerAction];

type ServerAction = ServerActionType | string;
type MessageHandler = (data: Record<string, unknown>) => void;

interface ConnectionStatus {
    status: "disconnected" | "connecting" | "connected";
    username: string;
    room: string;
}

let ws = $state<WebSocket | null>(null);
let connectionStatus = $state<ConnectionStatus>({
    status: "disconnected",
    username: "",
    room: "",
});

// Maps actions to sets of unique callbacks to call when receiving said action
const handlers = new Map<string, Set<MessageHandler>>();

function dispatch(data: Record<string, unknown>) {
    const action = data.action as string;

    // Call specific action handlers
    handlers.get(action)?.forEach((h) => h(data));
    // Call wildcard handlers
    handlers.get("*")?.forEach((h) => h(data));
}

/**
 * Register a handler for a specific action or all actions
 * Returns cleanup function
 */
export function on(action: ServerAction | "*", handler: MessageHandler) {
    if (!handlers.has(action)) handlers.set(action, new Set());
    handlers.get(action)!.add(handler);

    return () => handlers.get(action)?.delete(handler);
}

/**
 * Connect to WebSocket server
 */
export async function connect(): Promise<void> {
    if (ws?.readyState === WebSocket.OPEN) return;

    const protocol = location.protocol === "https:" ? "wss:" : "ws:";
    const url = `${protocol}//${location.host}`;

    ws = new WebSocket(url);
    connectionStatus.status = "connecting";

    return new Promise((resolve, reject) => {
        ws!.onopen = () => {
            connectionStatus.status = "connected";
            resolve();
        };
        ws!.onerror = (event) => {
            reject(new Error("WebSocket connection failed"));
        };
        ws!.onclose = () => {
            connectionStatus.status = "disconnected";
            ws = null;
        };
        ws!.onmessage = (e: MessageEvent) => {
            try {
                const data = JSON.parse(e.data);
                if (data.action === "sync_data") {
                    connectionStatus.username = data.username || "";
                    connectionStatus.room = data.room || "";
                }
                dispatch(data);
            } catch {
                // Ignore non-JSON messages
            }
        };
    });
}

/**
 * Disconnect from WebSocket server
 */
export function disconnect(code: number, reason: string): void {
    if (ws) {
        ws.close(code, reason);
        ws = null;
        connectionStatus.status = "disconnected";
    }
}

/**
 * Emit a message to the server
 */
export function emit(
    action: string,
    payload: Record<string, unknown> = {},
): void {
    if (ws?.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ action, ...payload }));
    }
}

/**
 * Emit a message and wait for a specific response action
 */
export async function emitAndWait(
    action: string,
    payload: Record<string, unknown> = {},
    expectedAction: string,
    timeoutMs = 5000,
): Promise<Record<string, unknown>> {
    return new Promise((resolve, reject) => {
        const timer = setTimeout(() => {
            cleanup();
            reject(new Error(`Timeout waiting for ${expectedAction}`));
        }, timeoutMs);

        const cleanup = on(expectedAction, (data) => {
            clearTimeout(timer);
            cleanup();
            resolve(data);
        });

        emit(action, payload);
    });
}

/**
 * Check if WebSocket is connected
 */
export function isConnected(): boolean {
    return ws?.readyState === WebSocket.OPEN;
}

/**
 * Get current connection status (reactive)
 */
export function getConnectionStatus(): Readonly<ConnectionStatus> {
    return connectionStatus;
}
