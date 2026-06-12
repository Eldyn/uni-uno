#pragma once

#include <webserver.hpp>
#include <controllers/lobby_controller.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <functional>
#include <cstdint>

/**
 * @file game_controller.hpp
 * @brief Controller per la gestione degli eventi in-game e del flusso dei turni.
 * * Smista i messaggi WebSocket riguardanti le azioni di gioco (giocare carte,
 * pescare, dichiarare UNO) alle rispettive istanze di MatchInstance.
 */

/**
 * @class GameController
 * @brief Riceve e processa l'input dei giocatori durante una partita attiva.
 * * Lavora a stretto contatto con il `LobbyController` per identificare la partita 
 * associata all'utente. Questa classe è anche responsabile del ciclo di vita dei 
 * timer dei turni (libuv) per gestire i giocatori AFK o il takeover dei bot.
 * @tag CTRL-GAME-001
 */
class GameController {
public:
    /**
     * @brief Costruttore del controller di gioco.
     * Registra gli handler delle azioni WebSocket `game_*` sull'ActionRouter.
     * @param server L'istanza principale del WebServer.
     * @param lobby_controller Riferimento al controller delle lobby per recuperare lo stato.
     * @tag CTRL-GAME-MTH-001
     */
    GameController(WebServer& server, LobbyController& lobby_controller);

private:
    ActionRouter& action_router_;       /**< Riferimento al router WebSocket. */
    LobbyController& lobby_controller_; /**< Riferimento per accedere alle lobby in memoria. */

    /** * @brief Mappa che associa l'ID di una Lobby al puntatore del timer libuv corrente.
     * Utilizzato per attivare la protezione AFK (es. il bot subentra o la mossa viene saltata).
     */
    std::unordered_map<uint32_t, struct us_timer_t*> active_turn_timers_;

    // --- WebSocket Event Handlers ---

    /**
     * @brief Gestisce la richiesta di giocare una carta.
     * @param context Contesto del socket chiamante.
     * @param message Payload JSON contenente il campo `card_id`.
     * @tag CTRL-GAME-ACT-001
     */
    void HandlePlayCard(WsContext context, const nlohmann::json& message);

    /**
     * @brief Gestisce la richiesta di pescare una carta dal mazzo.
     * @param context Contesto del socket chiamante.
     * @param message Payload JSON della richiesta.
     * @tag CTRL-GAME-ACT-002
     */
    void HandleDrawCard(WsContext context, const nlohmann::json& message);

    /**
     * @brief Gestisce l'invio di un input da parte del client per un effetto in sospeso (es. scelta colore).
     * @param context Contesto del socket chiamante.
     * @param message Payload JSON contenente il dato scelto.
     * @tag CTRL-GAME-ACT-003
     */
    void HandleProvideInput(WsContext context, const nlohmann::json& message);

    /**
     * @brief Gestisce la dichiarazione di "UNO" da parte del giocatore.
     * @param context Contesto del socket chiamante.
     * @param message Payload JSON della richiesta.
     * @tag CTRL-GAME-ACT-004
     */
    void HandleCallUno(WsContext context, const nlohmann::json& message);
    
    // --- Core Game Flow ---

    /**
     * @brief Callback/Hook scatenato all'inizio di ogni nuovo turno di un giocatore.
     * Avvia/reimposta il timer per l'AFK in base alle impostazioni della lobby.
     * @param active_lobby Puntatore alla lobby di cui è iniziato il turno.
     * @tag CTRL-GAME-FLW-001
     */
    void OnTurnStarted(Lobby* active_lobby);

    /**
     * @brief Invia lo stato del gioco censurato a tutti i membri connessi.
     * Iterando su ogni utente, usa `MatchInstance::SerializePlayerState` per
     * nascondere le mani avversarie e inoltra il messaggio WS.
     * @param current_lobby Puntatore alla lobby da aggiornare.
     * @tag CTRL-GAME-FLW-002
     */
    void BroadcastGameState(Lobby* current_lobby);

    // --- Timer Management Helpers ---

    /**
     * @brief Crea o aggiorna il timer del turno per la protezione AFK.
     * @param lobby_id ID della lobby in corso.
     * @param timeout_ms Millisecondi prima dello scatto del timeout (es. 15000ms).
     * @param callback Funzione da eseguire alla scadenza del timer (es. auto-play o bot).
     * @tag CTRL-GAME-TMR-001
     */
    void SetTurnTimer(uint32_t lobby_id, int timeout_ms, std::function<void()> callback);

    /**
     * @brief Ferma e distrugge il timer attivo per una determinata lobby.
     * Usato quando un giocatore esegue un'azione valida in tempo o la partita finisce.
     * @param lobby_id ID della lobby di cui cancellare il timer.
     * @tag CTRL-GAME-TMR-002
     */
    void ClearTurnTimer(uint32_t lobby_id);
};
