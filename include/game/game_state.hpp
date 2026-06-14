#pragma once

#include <common/game/card_types.hpp>
#include <common/game/effect.hpp>
#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

/**
 * @file game_state.hpp
 * @brief Strutture dati fondazionali per la rappresentazione dello stato del gioco "UNO".
 * * Contiene i dati puri disaccoppiati dalla logica di instradamento. Lo stato del gioco è
 * il contesto su cui agiscono gli Effetti (Effect) e i Validatori delle regole (GameRule).
 */

namespace game {
    
    /**
     * @struct PlayerSessionStats
     * @brief Traccia le statistiche delle carte giocate/pescate in una singola sessione.
     * @tag GAME-STRUCT-001
     */
    struct PlayerSessionStats {
        int color_counts[5] = {0};  /**< Conteggio frequenze per i vari Colori (incluso Wild). */
        int value_counts[15] = {0}; /**< Conteggio frequenze per i Valori (0-9, e carte speciali). */
    };

    /**
     * @struct Player
     * @brief Rappresenta un giocatore reale o Bot all'interno della partita in corso.
     * @tag GAME-STRUCT-002
     */
    struct Player {
        std::string username;           /**< Nome utente identificativo. */
        std::vector<CompactCard> hand;  /**< Le carte attualmente in mano al giocatore. */
        bool is_bot;                    /**< Flag che indica se il giocatore è guidato dalla CPU. */
        bool has_called_uno = false;    /**< Flag che indica se ha dichiarato "UNO" correttamente. */

        /**
         * @brief Costruttore completo del Player.
         * @param u Username.
         * @param h Mano iniziale di carte.
         * @param b True se Bot.
         * @param uno True se ha già chiamato UNO.
         */
        Player(std::string u, std::vector<CompactCard> h, bool b, bool uno) : username(u), hand(h), is_bot(b), has_called_uno(uno) {};
        
        /**
         * @brief Costruttore senza carte in mano (fase di preparazione).
         */
        Player(std::string u, bool b, bool uno) : username(u), is_bot(b), has_called_uno(uno) {};
    };
    
    /**
     * @enum MatchStatus
     * @brief Fasi del ciclo di vita globale di una partita.
     * @tag GAME-ENUM-001
     */
    enum class MatchStatus {
        kWaitingToStart, /**< La partita sta inizializzando i deck o è in pre-game. */
        kPlaying,        /**< Partita attivamente in corso (i turni scorrono). */
        kFinished        /**< Partita terminata. C'è un vincitore stabilito. */
    };

    /**
     * @struct LastPlay
     * @brief Descrive l'ultima carta giocata e la sua provenienza, così che il client
     * possa animare la carta dalla specifica posizione (slot) della mano di chi l'ha giocata.
     * @tag GAME-STRUCT-004
     */
    struct LastPlay {
        bool valid = false;       /**< True se contiene una giocata valida da animare. */
        std::string player;       /**< Username di chi ha giocato la carta. */
        int hand_index = -1;      /**< Indice nello slot della mano da cui è partita la carta. */
        CompactCard card{};       /**< La carta giocata (id/colore/valore compatti). */
    };

    /**
     * @struct GameState
     * @brief Struttura dati monolitica che contiene l'intera "fotografia" della partita ad un dato istante.
     * Viene manipolata dalle classi Effect e MatchInstance e serializzata in JSON.
     * @tag GAME-STRUCT-003
     */
    struct GameState {
        MatchStatus status = MatchStatus::kWaitingToStart; /**< Lo stato di progressione. */
        std::string winner;                                /**< Username del vincitore (se conclusa). */
    
        std::vector<Player> players;                       /**< Lista ordinata (per turno) dei giocatori. */
        int current_player_index = 0;                      /**< Indice del giocatore con il turno attivo. */
        int play_direction = 1;                            /**< Direzione di gioco (1 = orario, -1 = antiorario). */
        int pending_draws = 0;                             /**< Carte accumulate che il prossimo giocatore dovrà pescare (es. catena +2/+4). */
        LastPlay last_play;                                /**< Provenienza dell'ultima carta giocata, usata per le animazioni client. */

        std::vector<CompactCard> draw_pile;                /**< Mazzo delle carte da cui pescare. */
        std::vector<CompactCard> discard_pile;             /**< Mazzo degli scarti al centro del tavolo. */
        Color active_color = Color::kRed;                  /**< Colore attualmente obbligatorio per rispondere (cambia con carte Jolly). */
        
        std::deque<std::unique_ptr<Effect>> effect_queue;  /**< Coda di effetti da risolvere (Architecture asincrona per mosse a catena). */
        
        std::string pending_player;                        /**< L'utente che DEVE fornire un input prima che il gioco continui. */
        std::string pending_input_type;                    /**< Tipo di input richiesto (es. "choose_color"). */
        std::string pending_input_context;                 /**< Payload JSON aggiuntivo per aiutare il client nel rendering dell'input box. */
        std::string provided_input;                        /**< Risposta asincrona memorizzata appena inviata dal giocatore per l'effetto. */

        std::chrono::steady_clock::time_point turn_end_time; /**< Timestamp in cui scatterà l'AFK/Timeout del turno corrente. */
    };

    /**
     * @brief Utilità di sistema che sposta e rimescola le carte dagli scarti al mazzo principale.
     * Viene invocata automaticamente dal motore di gioco quando `draw_pile` si esaurisce.
     * @param game_state Puntatore allo stato di gioco da manipolare.
     * @tag GAME-UTIL-001
     */
    void ReshuffleDiscardIntoDraw(GameState* game_state);
}
