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
    ChatSend: "chat_send"
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
    ChatMessage: "chat_message"
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
    room: ""
});

let _nextRequestId = 1;
function nextRequestId(): string {
    return String(_nextRequestId++);
}

// Maps actions to sets of unique callbacks to call when receiving said action
const handlers = new Map<string, Set<MessageHandler>>();

interface PendingRequests {
    resolve: (data: Record<string, unknown>) => void;
    reject: (err: Error) => void;
    timer: ReturnType<typeof setTimeout>;
}

const pendingRequests = new Map<string, PendingRequests>();

function dispatch(data: Record<string, unknown>) {
    const action = data.action as string;
    const requestId = data.request_id as string;

    if (requestId && pendingRequests.has(requestId)) {
        const pending = pendingRequests.get(requestId)!;
        pendingRequests.delete(requestId);
        clearTimeout(pending.timer);

        if (action === "error") {
            pending.reject(new Error(data.reason as string)) ?? "Server Error";
        } else {
            pending.resolve(data);
        }
    }

    if (action) {
        handlers.get(action)?.forEach((handler) => handler(data));
        handlers.get("*")?.forEach((handler) => handler(data));
    }
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

let reconnectTimer: ReturnType<typeof setTimeout> | null;
let reconnectDelayMs = 1000;
let intentionalClose = false;

function scheduleReconect() {
    if (intentionalClose) return;
    if (reconnectTimer) clearTimeout(reconnectTimer);

    reconnectTimer = setTimeout(() => {
        reconnectTimer = null;
        reconnectDelayMs = Math.min(reconnectDelayMs * 2, 16_000);

        connectOnce().catch(() => {
            scheduleReconect();
        });
    });
}

function connectOnce(): Promise<void> {
    return new Promise((resolve, reject) => {
        const protocol = location.protocol === "https:" ? "wss:" : "ws:";
        const socket = new WebSocket(`${protocol}//${location.host}`);
        connectionStatus.status = "connecting";

        socket.onopen = () => {
            ws = socket;
            connectionStatus.status = "connected";
            reconnectDelayMs = 1000;
            resolve();

            // Re-join lobby if we were in one before the disconnect.
            const savedCode = sessionStorage.getItem("lobby_code");
            if (savedCode) emit("lobby_rejoin", { code: savedCode });
        };

        socket.onerror = (e) => reject(e);

        socket.onclose = () => {
            if (ws === socket) {
                ws = null;
                connectionStatus.status = "disconnected";
            }
            scheduleReconect();
        };

        socket.onmessage = (e: MessageEvent) => {
            try {
                const data = JSON.parse(e.data as string) as Record<string, unknown>;
                if (data.action === "sync_data") connectionStatus.username = data.username as string;
                dispatch(data);
            } catch {
                /* ignore non-JSON */
            }
        };
    });
}

// ── Public API ────────────────────────────────────────────────────────────────

export async function connect(): Promise<void> {
    if (ws?.readyState === WebSocket.OPEN) return;
    intentionalClose = false;
    await connectOnce();
}

export function disconnect(): void {
    intentionalClose = true;
    if (reconnectTimer) {
        clearTimeout(reconnectTimer);
        reconnectTimer = null;
    }
    ws?.close();
    ws = null;
    connectionStatus.status = "disconnected";
}

// emit — fire and forget, no response expected.
// Does NOT attach a request_id so the server won't try to correlate it.
export function emit(action: string, payload: object = {}): void {
    if (ws?.readyState === WebSocket.OPEN) ws.send(JSON.stringify({ action, ...payload }));
}

// emitAndWait — send a message and wait for the correlated response.
//
// Attaches a unique request_id.  The server must echo it back.
// Rejects if:
//   - The server sends an "error" response with the same request_id
//   - No response arrives within timeoutMs
//
// The `expectedAction` parameter is now OPTIONAL documentation — it is no
// longer used for matching.  Matching is done exclusively on request_id.
// You can still pass it for clarity, but removing it won't break anything.
export function emitAndWait(
    action: string,
    payload: object = {},
    _expectedAction?: string, // kept for API compatibility, not used for matching
    timeoutMs = 5000
): Promise<Record<string, unknown>> {
    return new Promise((resolve, reject) => {
        const requestId = nextRequestId();

        const timer = setTimeout(() => {
            pendingRequests.delete(requestId);
            reject(new Error(`Timeout: no response for "${action}" (request_id=${requestId})`));
        }, timeoutMs);

        pendingRequests.set(requestId, { resolve, reject, timer });

        if (ws?.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({ action, request_id: requestId, ...payload }));
        } else {
            // Socket not open — reject immediately rather than silently timing out.
            pendingRequests.delete(requestId);
            clearTimeout(timer);
            reject(new Error("WebSocket not connected"));
        }
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
