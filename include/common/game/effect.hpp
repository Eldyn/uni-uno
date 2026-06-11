#pragma once

#include <string>

/**
 * @file include/common/game/effect.hpp
 * @brief Sistema di base per la definizione e la risoluzione degli effetti di gioco.
 *
 * @tag FEAT-CG-002 Gestione Asincrona degli Effetti (Effect Engine): Architettura polimorfica basata su eventi e stati per elaborare e sospendere la risoluzione degli effetti in attesa di input dell'utente.
 */

namespace game {

    struct GameState;
    class MatchInstance;

    /**
     * @enum EffectStatus
     * @brief Enumera i possibili esiti della risoluzione di un effetto.
     */
    enum class EffectStatus { 
        kResolved,   /**< L'effetto è stato applicato con successo e completato. */
        kNeedsInput, /**< L'effetto è sospeso in attesa di un input da un giocatore (es. scelta colore). */
        kError       /**< Si è verificato un errore durante la risoluzione. */
    };
    
    /**
     * @struct EffectResult
     * @brief Struttura restituita dopo il tentativo di risoluzione di un effetto.
     */
    struct EffectResult {
        EffectStatus status;             /**< Lo stato di risoluzione dell'effetto. */
        std::string input_type;          /**< Il tipo di input richiesto (se status == kNeedsInput). */
        std::string target_player;       /**< Il giocatore da cui si attende l'input. */
        std::string input_context = "";  /**< Un payload JSON generico con informazioni contestuali per il frontend. */
    };

    /**
     * @enum EffectType
     * @brief Codici per l'identificazione a runtime del tipo di effetto in corso.
     */
    enum class EffectType {
        kAdvanceTurn,       /**< Passaggio del turno al giocatore successivo */
        kDraw,              /**< Obbligo di pescare carte */
        kSkip,              /**< Salto del turno */
        kPlayCard,          /**< Giocata di una carta */
        kDecideDrawnCard,   /**< Decisione su una carta appena pescata (giocare o tenere) */
        kChooseColor,       /**< Scelta di un nuovo colore attivo (es. dopo un Jolly) */
        kReverse,           /**< Inversione dell'ordine di gioco */
        kDecideSwapTarget,  /**< Scelta del target con cui scambiare le mani */
        kPassHands,         /**< Effetto per il passaggio o scambio delle mani tra i giocatori */
        kCustom             /**< Effetto personalizzato (per mod o estensioni) */
    };

    /**
     * @class Effect
     * @brief Classe base astratta per tutti gli effetti applicabili nel gioco.
     */
    class Effect {
    public:
        virtual ~Effect() = default;
        
        /**
         * @brief Recupera il tipo dell'effetto implementato.
         * @return EffectType Identificatore dell'effetto (default a kCustom).
         */
        virtual EffectType GetType() const { return EffectType::kCustom; }

        /**
         * @brief Esegue e applica l'effetto allo stato del gioco attuale.
         * @param state Puntatore allo stato globale della partita (GameState).
         * @param match Puntatore all'istanza della partita (MatchInstance) per coordinare input/bot.
         * @return EffectResult Il risultato dell'applicazione (risolto, errore o necessita input).
         */
        virtual EffectResult Resolve(GameState* state, MatchInstance* match) = 0;
    };

}
