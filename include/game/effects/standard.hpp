#pragma once
#include <common/game/effect.hpp>
#include <string>
#include <cstdint>

/**
 * @file standard.hpp
 * @brief Implementazione degli effetti standard associati alle carte di base.
 * * Questo file definisce l'insieme degli effetti classici (es. Salto del turno, Inversione,
 * Pesca carte, Scelta colore) che vengono accodati e risolti dal motore di gioco durante una partita.
 */

namespace game {

    /**
     * @class AdvanceTurnEffect
     * @brief Effetto base che avanza il turno al giocatore successivo.
     * Viene solitamente accodato alla fine di una giocata standard o dopo la risoluzione degli altri effetti.
     * @tag EFFECT-STD-001
     */
    class AdvanceTurnEffect : public Effect {
    public:
        /**
         * @brief Risolve l'effetto aggiornando l'indice del giocatore corrente nello stato di gioco.
         * @param game_state Lo stato attuale del gioco.
         * @param match_instance Istanza della partita.
         * @return EffectResult L'esito della risoluzione (sempre kResolved in assenza di errori).
         * @tag EFFECT-STD-MTH-001
         */
        EffectResult Resolve(GameState* game_state, MatchInstance* match_instance) override;

        /**
         * @brief Restituisce il tipo enumerativo dell'effetto.
         * @return EffectType Identificatore kAdvanceTurn.
         * @tag EFFECT-STD-MTH-002
         */
        EffectType GetType() const override { return EffectType::kAdvanceTurn; }
    };

    /**
     * @class DecideDrawnCardEffect
     * @brief Effetto interattivo che chiede al giocatore se vuole giocare la carta appena pescata.
     * * Si innesca quando un giocatore pesca una carta dal mazzo (non per penalità) e 
     * questa risulta essere giocabile secondo le regole correnti.
     * @tag EFFECT-STD-002
     */
    class DecideDrawnCardEffect : public Effect {
    public:
        /**
         * @brief Costruttore dell'effetto.
         * @param username Nome del giocatore che ha appena pescato.
         * @param card_id L'ID a 16-bit della carta appena pescata.
         */
        explicit DecideDrawnCardEffect(const std::string& username, uint16_t card_id) : username_(username), card_id_(card_id) {}
            
        /**
         * @brief Sospende il motore di gioco in attesa dell'input dell'utente.
         * @param state Lo stato del gioco.
         * @param match Istanza della partita.
         * @return EffectResult Restituisce uno stato kNeedsInput per chiedere al client cosa fare.
         * @tag EFFECT-STD-MTH-003
         */
        EffectResult Resolve(GameState* state, MatchInstance* match) override;

        /**
         * @brief Restituisce il tipo enumerativo dell'effetto.
         * @return EffectType Identificatore kDecideDrawnCard.
         * @tag EFFECT-STD-MTH-004
         */
        EffectType GetType() const override { return EffectType::kDecideDrawnCard; }
    private:
        std::string username_; /**< L'utente che deve prendere la decisione. */
        uint16_t card_id_;     /**< L'ID della carta su cui decidere. */
    };

    /**
     * @class DrawEffect
     * @brief Effetto penalità/obbligo che fa pescare un numero di carte a un giocatore.
     * Usato per l'inizio del turno, le carte +2, +4, o per penalità da mod.
     * @tag EFFECT-STD-003
     */
    class DrawEffect : public Effect {
    public:
        /**
         * @brief Costruttore dell'effetto di pescaggio.
         * @param count Il numero di carte da far pescare.
         * @param target_username L'utente che subirà l'effetto.
         */
        explicit DrawEffect(int count, const std::string& target_username) : count_(count), target_username_(target_username) {} 

        /**
         * @brief Risolve l'effetto prelevando le carte dal mazzo e aggiungendole alla mano del bersaglio.
         * Gestisce automaticamente il rimescolamento del mazzo degli scarti se necessario.
         * @param state Lo stato del gioco.
         * @param match_instance Istanza della partita.
         * @return EffectResult Esito dell'operazione.
         * @tag EFFECT-STD-MTH-005
         */
        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;

        /**
         * @brief Restituisce il tipo enumerativo dell'effetto.
         * @return EffectType Identificatore kDraw.
         * @tag EFFECT-STD-MTH-006
         */
        EffectType GetType() const override { return EffectType::kDraw; }
    private:
        int count_;                     /**< Quantità di carte. */
        std::string target_username_;   /**< Bersaglio dell'effetto. */
    };
    
    /**
     * @class SkipEffect
     * @brief Effetto che salta il turno del giocatore successivo (Carta Divieto).
     * @tag EFFECT-STD-004
     */
    class SkipEffect : public Effect {
    public:
        /**
         * @brief Avanza l'indice del giocatore di un passo addizionale, di fatto saltandolo.
         * @param state Lo stato del gioco.
         * @param match_instance Istanza della partita.
         * @return EffectResult Esito dell'operazione.
         * @tag EFFECT-STD-MTH-007
         */
        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;

        /**
         * @brief Restituisce il tipo enumerativo dell'effetto.
         * @return EffectType Identificatore kSkip.
         * @tag EFFECT-STD-MTH-008
         */
        EffectType GetType() const override { return EffectType::kSkip; }
    };
    
    /**
     * @class ReverseEffect
     * @brief Effetto che inverte il senso di rotazione dei turni (Carta Inverti).
     * @tag EFFECT-STD-005
     */
    class ReverseEffect : public Effect {
    public:
        /**
         * @brief Inverte la direzione di gioco (es. da 1 a -1 o viceversa). In 2 giocatori funge da Skip.
         * @param state Lo stato del gioco.
         * @param match_instance Istanza della partita.
         * @return EffectResult Esito dell'operazione.
         * @tag EFFECT-STD-MTH-009
         */
        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;

        /**
         * @brief Restituisce il tipo enumerativo dell'effetto.
         * @return EffectType Identificatore kReverse.
         * @tag EFFECT-STD-MTH-010
         */
        EffectType GetType() const override { return EffectType::kReverse; }
    };
    
    /**
     * @class ChooseColorEffect
     * @brief Effetto interattivo che impone la selezione del nuovo colore attivo (Carte Jolly).
     * @tag EFFECT-STD-006
     */
    class ChooseColorEffect : public Effect {
    public:
        /**
         * @brief Costruttore dell'effetto.
         * @param target_username Il giocatore che ha giocato il Jolly e deve scegliere il colore.
         */
        explicit ChooseColorEffect(const std::string& target_username) : target_username_(target_username) {}

        /**
         * @brief Sospende l'esecuzione in attesa dell'input dal target. Se l'input è già presente, aggiorna il colore globale.
         * @param state Lo stato del gioco.
         * @param match_instance Istanza della partita.
         * @return EffectResult Esito dell'operazione o kNeedsInput.
         * @tag EFFECT-STD-MTH-011
         */
        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;

        /**
         * @brief Restituisce il tipo enumerativo dell'effetto.
         * @return EffectType Identificatore kChooseColor.
         * @tag EFFECT-STD-MTH-012
         */
        EffectType GetType() const override { return EffectType::kChooseColor; }
    private:
        std::string target_username_; /**< Utente che deve fornire la scelta del colore. */
    };
}
