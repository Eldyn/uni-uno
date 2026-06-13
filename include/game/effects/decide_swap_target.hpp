#pragma once

#include <common/game/effect.hpp>
#include <string>

/**
 * @file decide_swap_target.hpp
 * @brief Effetto interattivo per la scelta del bersaglio per lo scambio delle carte.
 * * Richiede un'azione esplicita da parte di un giocatore per decidere con quale 
 * avversario scambiare la propria intera mano di carte (tipico della carta "7").
 */

namespace game {

    /**
     * @class DecideSwapTargetEffect
     * @brief Sospende il gioco per chiedere a un utente con chi desidera scambiare le carte.
     * * Questo effetto non scambia fisicamente le carte, ma raccoglie l'input dell'utente.
     * Una volta ricevuto l'input (il nome del target), tipicamente accoda un `PassHandsEffect` 
     * o un effetto simile mirato ai due giocatori coinvolti.
     * @tag EFFECT-SWAP-001
     */
    class DecideSwapTargetEffect : public Effect {
    public:
        /**
         * @brief Costruttore dell'effetto interattivo.
         * @param username Nome del giocatore a cui spetta la decisione.
         */
        explicit DecideSwapTargetEffect(std::string username) : username_(std::move(username)) {}

        /**
         * @brief Sospende l'esecuzione in attesa dell'input dal giocatore.
         * Se l'input (username avversario) è già stato fornito e memorizzato nello stato,
         * convalida la mossa e procede.
         * @param state Lo stato del gioco.
         * @param match Istanza della partita.
         * @return EffectResult Restituisce kNeedsInput per interrogare il client.
         * @tag EFFECT-SWAP-MTH-001
         */
        EffectResult Resolve(GameState* state, MatchInstance* match) override;

        /**
         * @brief Restituisce il tipo enumerativo dell'effetto.
         * @return EffectType Identificatore kDecideSwapTarget.
         * @tag EFFECT-SWAP-MTH-002
         */
        EffectType GetType() const override { return EffectType::kDecideSwapTarget; }

    private:
        std::string username_; /**< L'utente che deve selezionare il target. */
    };
}
