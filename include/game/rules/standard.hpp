#pragma once
#include <common/game/gamerule.hpp>

/**
 * @file standard.hpp
 * @brief Implementazione del set di regole classiche di base del gioco UNO.
 * * Questa regola è responsabile della validazione primaria (colore su colore,
 * valore su valore) e dell'inserimento nella coda di risoluzione degli effetti
 * associati ad ogni tipo di carta speciale giocata.
 */

namespace game {

   /**
    * @class StandardRule
    * @brief Validatore e processore principale delle meccaniche standard del gioco.
    * * Assicura che la mossa sia legale rispetto allo stato attuale del tavolo e,
    * se convalidata, decodifica il valore della carta instanziando gli Effetti 
    * corrispondenti (es. se la carta è un "Pesca Due", accoda un `DrawEffect` e 
    * un `AdvanceTurnEffect`).
    * @tag RULE-STD-001
    */
   class StandardRule : public GameRule {
    public:
        ~StandardRule() override = default;

        /**
         * @brief Intercetta il tentativo di giocata valutandone la legalità.
         * Verifica se la carta giocata corrisponde in colore o in valore/simbolo alla carta 
         * in cima agli scarti (o al colore attivo per i Jolly). Modifica `event.is_valid_play` in caso negativo.
         * @param state Puntatore allo stato di gioco attuale.
         * @param event Struttura evento contenente i dettagli della giocata.
         * @tag RULE-STD-MTH-001
         */
        void ValidatePlay(GameState* state, CardPlayedEvent& event) override;

        /**
         * @brief Esegue le conseguenze di una giocata confermata valida.
         * Mette in coda in `GameState::effect_queue` le classi `Effect` pertinenti basate
         * sull'enumeratore `Value` estratto dalla carta compatta giocata.
         * @param state Puntatore allo stato di gioco.
         * @param event L'evento della carta giocata.
         * @tag RULE-STD-MTH-002
         */
        void OnCardPlayed(GameState* state, CardPlayedEvent& event) override;

    private:
        /**
         * @brief Calcola chi sarà il giocatore a subire le penalità o il prossimo a giocare.
         * Metodo di supporto utilizzato internamente per determinare il target (es. per `SkipEffect` o `DrawEffect`).
         * * @param state Puntatore allo stato del gioco.
         * @return std::string Username del giocatore bersaglio calcolato seguendo la direzione di gioco.
         * @tag RULE-STD-PRIV-001
         */
        std::string GetNextPlayer(GameState* state);
   };
}
