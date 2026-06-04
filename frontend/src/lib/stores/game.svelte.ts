import { storeNavigation } from "./navigation.svelte";
import { ServerAction, ws } from "./ws.svelte";
import { storeAuth } from "./auth.svelte";

const COLOR_MAP = ["red", "blue", "green", "yellow", "wild"] as const;
const VALUE_MAP = [
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "Skip",
    "Rev",
    "+2",
    "Wild",
    "+4"
] as const;

export interface Card {
    id: number;
    color: string;
    value: string;
}

export interface GamePlayer {
    username: string;
    card_count: number;
    has_called_uno: boolean;
    hand?: Card[]; // Only populated for the local player!
}

export interface GameState {
    active_color: string;
    current_turn: string;
    play_direction: number;
    top_card?: Card;
    players: GamePlayer[];
}

class StoreGame {
    state = $state<GameState | null>(null);
    actionRequired = $state<string | null>(null);

    actionContext = $state<any>(null);

    turnTimeRemaining = $state<number>(15);
    #timerInterval: number | null = null;

    localPlayer = $derived(this.state?.players.find((p) => p.username === storeAuth.username));

    constructor() {
        ws.onOpen(() => this.#registerListeners());
    }

    #registerListeners() {
        ws.on(ServerAction.GameStateUpdated, (data: any) => {
            const previousTurn = this.state?.current_turn;
            const rawState = data.game_state;

            this.state = {
                active_color: COLOR_MAP[rawState.active_color] || "green",
                current_turn: rawState.current_turn,
                play_direction: rawState.play_direction,
                top_card: rawState.top_card ? this.#parseCard(rawState.top_card) : undefined,
                players: rawState.players.map((p: any) => ({
                    ...p,
                    hand: p.hand ? p.hand.map(this.#parseCard) : undefined
                }))
            };

            this.actionRequired = data.action_required || null;
            this.actionContext = data.action_context || null; // Capture the JSON context

            // Handle the Timer Syncing
            if (previousTurn !== this.state?.current_turn) {
                this.#resetTurnTimer();
            }

            if (storeNavigation.current === "lobby") {
                storeNavigation.goto("game");
            }
        });

        // we don't really care about sending toasts since too many errors would be sent
        // INFO: Listen for standard game errors (like playing out of turn)
        // ws.on("error", (data: any) => {
        //     storeToast.error(data.reason || "Invalid move!");
        // });
    }

    #resetTurnTimer() {
        // Clear any existing interval
        if (this.#timerInterval) {
            clearInterval(this.#timerInterval);
        }

        // Reset to your lobby's default time limit (e.g., 15 seconds)
        this.turnTimeRemaining = 15;

        // Start the local countdown
        this.#timerInterval = window.setInterval(() => {
            this.turnTimeRemaining -= 1;
            if (this.turnTimeRemaining <= 0) {
                clearInterval(this.#timerInterval!);
            }
        }, 1000);
    }

    #parseCard(rawCard: { id: number; color: number; value: number }): Card {
        return {
            id: rawCard.id,
            color: COLOR_MAP[rawCard.color] || "wild",
            value: VALUE_MAP[rawCard.value] || ""
        };
    }

    playCard(cardId: number) {
        ws.emit("game_play_card", { card_id: cardId });
    }

    drawCard() {
        ws.emit("game_draw_card");
    }

    callUno() {
        ws.emit("game_call_uno");
    }

    submitInput(value: string) {
        ws.emit("game_submit_input", { value: value.toUpperCase() });
        this.actionRequired = null;
    }
}

export const storeGame = new StoreGame();
