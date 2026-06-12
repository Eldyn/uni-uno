#pragma once
#include <common/ws.hpp>
#include <App.h>
#include <action_router.hpp>
#include <websocket_context.hpp>
#include <webserver.hpp>

/**
 * @file lobby.hpp
 * @brief Definisce le strutture dati base per la rappresentazione delle lobby.
 * * Disaccoppia lo stato dei dati puri (settings, membri, identificatori) 
 * dalla logica di gestione che risiede nel LobbyController.
 */

namespace game {
    class MatchInstance;
}

/**
 * @enum BotTakeoverMode
 * @brief Modalità di comportamento quando un bot subentra o viene sostituito.
 * @tag CMN-LOBBY-ENUM-001
 */
enum class BotTakeoverMode {
    kPlayInstantly,     /**< Il bot gioca istantaneamente appena è il suo turno. */
    kWaitUntilTurnEnd   /**< Il bot attende lo scadere del timer del turno prima di giocare. */
};

NLOHMANN_JSON_SERIALIZE_ENUM(BotTakeoverMode, {
    {BotTakeoverMode::kPlayInstantly, 0},
    {BotTakeoverMode::kWaitUntilTurnEnd, 1}
})

/**
 * @struct LobbySettings
 * @brief Contiene la configurazione dettagliata e le regole di una specifica lobby.
 * Queste impostazioni sono serializzabili in JSON per essere trasmesse ai client.
 * @tag CMN-LOBBY-STR-001
 */
struct LobbySettings {
    bool is_public = false;                 /**< Indica se la lobby appare nella lista pubblica. */
    int turn_time_limit_ms = 15'000;        /**< Limite di tempo per un turno (in millisecondi). */
    std::vector<std::string> active_mods;   /**< Mod o regole opzionali attive per la partita. */
    
    bool save_state = false;                /**< Se true, lo stato della partita viene salvato su DB. */
    bool quit_deletes_match = false;        /**< Se true, l'abbandono di un giocatore distrugge la partita salvata. */

    int bot_count = 0;                      /**< Numero di bot iniziali nella lobby. */
    BotTakeoverMode bot_mode = BotTakeoverMode::kWaitUntilTurnEnd; /**< Modalità di gioco dei bot. */
    
    bool allow_bot_takeover = true;         /**< Se true, un nuovo giocatore umano può rimpiazzare un bot a partita in corso. */
    bool allow_bot_replacement = true;      /**< Se true, un giocatore che esce viene sostituito da un bot anziché fermare il gioco. */

    int starting_cards       = 7;           /**< Numero di carte pescate all'inizio. */
    int count_zeros          = 1;           /**< Copie per colore del numero 0. */
    int count_numbered       = 2;           /**< Copie per colore dei numeri 1-9. */
    int count_skips          = 2;           /**< Copie per colore della carta "Salta". */
    int count_reverses       = 2;           /**< Copie per colore della carta "Inverti". */
    int count_draw_two       = 2;           /**< Copie per colore della carta "Pesca Due". */
    int count_wild           = 4;           /**< Numero totale di carte Jolly. */
    int count_wild_draw_four = 4;           /**< Numero totale di carte Jolly Pesca Quattro. */
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LobbySettings, 
    turn_time_limit_ms, active_mods, bot_count, bot_mode, starting_cards,
    allow_bot_takeover, allow_bot_replacement, quit_deletes_match, save_state, is_public
)

/**
 * @struct LobbyMember
 * @brief Rappresenta un singolo partecipante all'interno di una lobby.
 * @tag CMN-LOBBY-STR-002
 */
struct LobbyMember {
    std::string     username;      /**< Nome utente del membro. */
    AppWebSocket* socket;        /**< Puntatore al socket uWS. nullptr se l'utente è temporaneamente disconnesso. */
    bool            is_connected;  /**< Stato di connessione (true = online, false = offline in grace period). */
    bool            is_bot;        /**< True se questo "membro" è controllato dall'AI. */
    
    std::chrono::steady_clock::time_point disconnected_at{}; /**< Timestamp dell'ultima disconnessione (per il timer di espulsione). */

    LobbyMember(std::string u, AppWebSocket* s, bool c, bool b) : username(std::move(u)), socket(s), is_connected(c), is_bot(b)  {}
};

/**
 * @struct Lobby
 * @brief Aggrega l'intero stato strutturale di una stanza di gioco.
 * @tag CMN-LOBBY-STR-003
 */
struct Lobby {
    uint32_t                 id;            /**< Identificativo numerico interno univoco della lobby. */
    std::string              invite_code;   /**< Codice di invito a 6 caratteri alfanumerici. */
    std::string              host;          /**< Username del creatore/host attuale della lobby. */
    std::vector<LobbyMember> members;       /**< Lista dei partecipanti (Umani e Bot). */
    std::string              name;          /**< Nome testuale della lobby (per visualizzazione in lista). */
    LobbySettings            settings;      /**< Impostazioni attuali della partita in questa lobby. */

    /** * @brief Istanza del motore di gioco. 
     * Se nullptr, la lobby è in fase di attesa. Se popolato, una partita è correntemente in corso.
     */
    std::unique_ptr<game::MatchInstance> match;
};
