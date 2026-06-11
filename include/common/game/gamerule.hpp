#pragma once

#include <common/game/card_types.hpp>
#include <game/game_state.hpp>

/**
 * @file include/common/game/gamerule.hpp
 * @brief Definisce il motore base per l'applicazione delle regole del gioco.
 *
 * @tag FEAT-CG-003 Validazione Flessibile delle Regole (Rule Engine): Intercettazione delle giocate tramite eventi dedicati (CardPlayedEvent) per autorizzare, respingere o estendere le meccaniche di gioco standard.
 */

namespace game {
    
    /**
     * @struct CardPlayedEvent
     * @brief Struttura che incapsula i dati riguardanti l'evento di una carta messa in gioco.
     */
    struct CardPlayedEvent {
        std::string player_username;  /**< Il nome utente del giocatore che sta tentando la giocata. */
        CompactCard played_card;      /**< La carta che viene giocata. */

        bool is_valid_play = true;    /**< Flag che indica se la giocata è valida o va rigettata (impostabile dai validatori). */
        bool is_handled = false;      /**< Flag che indica se l'evento è già stato completamente processato da una regola. */
        bool is_out_of_turn = false;  /**< Flag che indica se la giocata è avvenuta fuori turno (utile per regole speciali o penalità). */
    };
    
    /**
     * @class GameRule
     * @brief Interfaccia base che definisce il ciclo di vita per l'interpretazione di una giocata.
     */
    class GameRule {
    public:
        virtual ~GameRule() = default;
    
        /**
         * @brief Valuta se la carta giocata rispetta le regole imposte.
         * Viene chiamato prima che la carta venga effettivamente accettata dal motore.
         * @param state Lo stato attuale del gioco.
         * @param event L'evento che porta i dati della giocata. Il metodo dovrebbe impostare event.is_valid_play a false se la mossa è illegale.
         */
        virtual void ValidatePlay(GameState* state, CardPlayedEvent& event) {}
        
        /**
         * @brief Scatena le conseguenze derivanti da una giocata convalidata.
         * Viene chiamato solo se ValidatePlay ha permesso la giocata. Tipicamente usato per istanziare ed accodare
         * le istanze figlie di `Effect` nella coda degli effetti di `GameState`.
         * @param state Lo stato attuale del gioco.
         * @param event L'evento contenente la giocata risolta.
         */
        virtual void OnCardPlayed(GameState* state, CardPlayedEvent& event) {}
    };
}