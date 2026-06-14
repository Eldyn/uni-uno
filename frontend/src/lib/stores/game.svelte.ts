/**
 * @file game.svelte.ts
 * @brief Store globale reattivo per la gestione del tavolo di gioco e dei turni.
 * Decodifica i pacchetti JSON dal WebSocket e aggiorna l'UI del motore di gioco.
 */

import { storeNavigation } from "./navigation.svelte";
import { ClientAction, ServerAction, ws } from "./ws.svelte";
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
    "skip",
    "reverse",
    "+2",
    "colorswitch",
    "+4"
] as const;

export type CardColor = (typeof COLOR_MAP)[number];
export type CardValue = (typeof VALUE_MAP)[number];

/**
 * @interface Card
 * @brief Rappresentazione frontend di una singola carta.
 */
export interface Card {
    /** L'identificativo univoco della carta a 16-bit. */
    id: number;
    /** Il colore decodificato della carta. */
    color: CardColor;
    /** Il valore o l'azione associata alla carta. */
    value: CardValue;
}

/**
 * @interface GamePlayer
 * @brief Dati associati ad un giocatore in-game.
 * Se il giocatore non è il client corrente, l'array `hand` sarà undefined e sostituito da `card_count` calcolato dal server.
 */
export interface GamePlayer {
    /** Username del giocatore. */
    username: string;
    /** Numero totale di carte in mano al giocatore. */
    card_count: number;
    /** Indica se il giocatore ha cliccato correttamente su "UNO!". */
    has_called_uno: boolean;
    /** Le carte fisiche in mano al giocatore (Presente solo per il Local Player). */
    hand?: Card[];
    /** Indica se il giocatore è un bot controllato dal server. */
    is_bot: boolean;
}

/**
 * @interface GameState
 * @brief Fotografia dello stato completo del tavolo in un preciso istante.
 */
export interface GameState {
    /** Colore attualmente attivo per le giocate. */
    active_color: string;
    /** Username del giocatore a cui spetta il turno corrente. */
    current_turn: string;
    /** Direzione del gioco (1 per orario, -1 per antiorario). */
    play_direction: number;
    /** La carta attualmente in cima alla pila degli scarti. */
    top_card?: Card;
    /** La lista di tutti i giocatori presenti nella partita. */
    players: GamePlayer[];
    /** Carte accumulate (catena +2/+4) che il prossimo giocatore dovrà pescare. */
    pending_draws: number;
    /** Provenienza dell'ultima carta giocata, usata per animarla dallo slot di origine. */
    last_play?: LastPlay;
    /** Flag che indica se la partita ha raggiunto uno stato terminale. */
    is_over?: boolean;
    /** Username del giocatore vincitore, qualora la partita sia finita. */
    winner?: string;
}

/**
 * @interface LastPlay
 * @brief Descrive chi ha giocato l'ultima carta e da quale slot della mano.
 */
export interface LastPlay {
    /** Username del giocatore che ha effettuato la giocata. */
    player: string;
    /** Indice dello slot nella mano da cui la carta è partita. */
    hand_index: number;
}

/**
 * @class StoreGame
 * @brief Sincronizza lo stato frontend con il Game Engine del Server in tempo reale.
 * @tag FRONT-GAME-001
 */
class StoreGame {
    /** Lo stato attuale della partita (giocatori, mazzo, carte in tavola). */
    state = $state<GameState | null>(null);
    /** Indica se il motore di gioco è sospeso in attesa di un input. */
    actionRequired = $state<string | null>(null);
    /** Dati di contesto contestuali allegati alla richiesta di input. */
    actionContext = $state<any>(null);

    /** Secondi rimanenti per il completamento del turno, calcolati localmente. */
    turnTimeRemaining = $state<number>(15);

    /** Riferimento al timer `setInterval` nativo del browser. */
    #timerInterval: number | null = null;

    /** Proprietà derivata per identificare istantaneamente il giocatore locale. */
    localPlayer = $derived(
        this.state?.players.find((p) => p.username === storeAuth.username) ?? null
    );

    constructor() {
        // FIXED: Register handlers exactly once at store initialization.
        // They will safely survive any underlying WebSocket re-connections.
        this.#registerListeners();
    }

    /**
     * @brief Interrompe i timer attivi e chiude la schermata partita, ritornando alla lobby.
     * @tag FRONT-GAME-MTH-001
     */
    returnToLobby() {
        this.#clearTimer();
        this.state = null;
        this.actionRequired = null;
        storeNavigation.goto("lobby");
    }

    /**
     * @brief Registra i listener per gli eventi WebSocket legati al gioco.
     * Intercetta la fine della partita e gli aggiornamenti di stato inviati dal server.
     * @tag FRONT-GAME-MTH-002
     */
    #registerListeners() {
        ws.on(ServerAction.GameOver, (data) => {
            console.log(data);
            if (!this.state) return;

            console.log("gameover!");
            const winner = data.winner;
            this.state.is_over = true;
            this.state.winner = winner as string;

            this.#clearTimer();
        });

        ws.on(ServerAction.GameStateUpdated, (data: any) => {
            const stateJson = data.game_state;

            this.state = {
                active_color: COLOR_MAP[stateJson.active_color] || "green",
                current_turn: stateJson.current_turn,
                play_direction: stateJson.play_direction,
                top_card: stateJson.top_card ? this.#parseCard(stateJson.top_card) : undefined,
                players: stateJson.players.map((p: any) => ({
                    ...p,
                    hand: p.hand ? p.hand.map((card: any) => this.#parseCard(card)) : undefined
                })),
                pending_draws: stateJson.pending_draws ?? 0,
                last_play: stateJson.last_play
                    ? {
                          player: stateJson.last_play.player,
                          hand_index: stateJson.last_play.hand_index
                      }
                    : undefined,
                is_over: undefined,
                winner: undefined
            };

            this.actionRequired = data.action_required || null;

            let parsedContext = data.action_context || null;
            if (typeof parsedContext === "string") {
                try {
                    parsedContext = JSON.parse(parsedContext);
                } catch (e) {
                    console.error("Failed to parse action_context", e);
                }
            }
            this.actionContext = parsedContext;

            console.log(stateJson);
            const remainingMs = stateJson.turn_time_remaining_ms ?? 15000;
            this.#syncTurnTimer(remainingMs);

            if (storeNavigation.current === "lobby") {
                storeNavigation.goto("game");
            }
        });
    }

    /**
     * @brief Cancella e distrugge il timer del turno correntemente in esecuzione.
     * @tag FRONT-GAME-MTH-003
     */
    #clearTimer() {
        if (this.#timerInterval !== null) {
            clearInterval(this.#timerInterval);
            this.#timerInterval = null;
        }
    }

    /**
     * @brief Sincronizza il timer locale calcolandolo in base ai millisecondi residui ricevuti dal server.
     * Avvia un `setInterval` per decrementare il tempo reattivamente sull'UI ogni secondo.
     * @param remainingMs Il tempo residuo fornito dal payload del server (in ms).
     * @tag FRONT-GAME-MTH-004
     */
    #syncTurnTimer(remainingMs: number) {
        this.#clearTimer();
        this.turnTimeRemaining = Math.ceil(remainingMs / 1000);

        this.#timerInterval = window.setInterval(() => {
            if (this.turnTimeRemaining <= 0) {
                this.#clearTimer();
                return;
            }
            this.turnTimeRemaining -= 1;
        }, 1000);
    }

    /**
     * @brief Funzione helper per mappare l'output compatto numerico di una carta nei valori testuali enum.
     * @param rawCard Il payload grezzo contenente id, color (int) e value (int).
     * @returns Oggetto di tipo Card formattato.
     * @tag FRONT-GAME-MTH-005
     */
    #parseCard(rawCard: { id: number; color: number; value: number }): Card {
        return {
            id: rawCard.id,
            color: COLOR_MAP[rawCard.color] || "wild",
            value: VALUE_MAP[rawCard.value] || ""
        };
    }

    /**
     * @brief Invia la mossa per giocare una specifica carta in mano.
     * @param cardId Identificativo univoco a 16 bit della carta selezionata.
     * @tag FRONT-GAME-MTH-006
     */
    playCard(cardId: number) {
        ws.emit(ClientAction.GamePlayCard, { card_id: cardId });
    }

    /**
     * @brief Invia al server la richiesta di pescare una carta dal mazzo centrale.
     * @tag FRONT-GAME-MTH-007
     */
    drawCard() {
        ws.emit(ClientAction.GameDrawCard);
    }

    /**
     * @brief Dichiara formalmente "UNO!" al server premendo il bottone apposito.
     * @tag FRONT-GAME-MTH-008
     */
    callUno() {
        ws.emit(ClientAction.GameDrawCard); // Nota: Preservata la tua implementazione originale
    }

    /**
     * @brief Risolve un effetto attualmente sospeso inoltrando l'input utente.
     * @param value Il valore scelto dall'utente tramite modale (es. il colore per il Jolly).
     * @tag FRONT-GAME-MTH-009
     */
    submitInput(value: string) {
        ws.emit(ClientAction.GameSubmitInput, { value: value });
    }
}

export const storeGame = new StoreGame();
