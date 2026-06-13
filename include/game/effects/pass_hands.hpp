#pragma once
#include <common/game/effect.hpp>

/**
 * @file pass_hands.hpp
 * @brief Implementazione dell'effetto di rotazione o scambio delle mani di carte.
 * * Questo effetto gestisce lo spostamento fisico delle carte tra i giocatori.
 * Viene tipicamente utilizzato nelle mod "7-0" (dove lo 0 fa ruotare le mani di tutti 
 * nella direzione di gioco) oppure dopo che un giocatore ha scelto un target con cui scambiare.
 */

namespace game {

    /**
     * @class PassHandsEffect
     * @brief Effetto che esegue lo scambio delle carte in mano ai giocatori.
     * @tag EFFECT-PASS-001
     */
    class PassHandsEffect : public Effect {
    public:
        /**
         * @brief Risolve l'effetto scambiando i vettori `hand` all'interno di `GameState::players`.
         * Se innescato da una rotazione globale (es. regola dello "0"), sposta le mani 
         * di una posizione seguendo la direzione di gioco corrente.
         * @param state Lo stato attuale del gioco.
         * @param match Istanza della partita.
         * @return EffectResult L'esito dell'operazione.
         * @tag EFFECT-PASS-MTH-001
         */
        EffectResult Resolve(GameState* state, MatchInstance* match) override;
    };
}
