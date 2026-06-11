#pragma once

#include <cstdint>

/**
 * @file include/common/game/card_types.hpp
 * @brief Definisce le strutture dati fondamentali e i tipi per la rappresentazione delle carte nel gioco.
 *
 * @tag FEAT-CG-001 Rappresentazione Compatta delle Carte: Utilizzo di un formato intero a 32-bit (CompactCard) per memorizzare simultaneamente colore, valore e ID univoco della carta.
 */

namespace game {

    /**
     * @brief Definizione del tipo CompactCard.
     * Impacchetta le informazioni di una carta all'interno di un intero a 32 bit.
     * Formato binario: `ccccccccvvvvvvvvuuuuuuuuuuuuuuuu`
     * - c (8 bit): Colore
     * - v (8 bit): Valore
     * - u (16 bit): ID univoco della carta
     */
    using CompactCard = uint32_t;

    /**
     * @enum Color
     * @brief Enumera i possibili colori che una carta può assumere.
     */
    enum class Color : uint8_t { 
        kRed = 0,    /**< Colore Rosso */
        kBlue = 1,   /**< Colore Blu */
        kGreen = 2,  /**< Colore Verde */
        kYellow = 3, /**< Colore Giallo */
        kWild = 4    /**< Colore Jolly (Nessun colore base) */
    };

    /**
     * @enum Value
     * @brief Enumera i possibili valori o azioni associati a una carta.
     */
    enum class Value : uint8_t { 
        k0 = 0, k1, k2, k3, k4, k5, k6, k7, k8, k9, /**< Valori numerici standard da 0 a 9 */
        kSkip = 10,       /**< Carta Salta Turno */
        kReverse = 11,    /**< Carta Inverti Giro */
        kDraw2 = 12,      /**< Carta Pesca Due */
        kWild = 13,       /**< Carta Jolly Cambia Colore */
        kWildDraw4 = 14   /**< Carta Jolly Pesca Quattro */
    };

    /**
     * @brief Estrae il colore da una carta compatta (Primi 8 bit).
     * @param c La carta compatta da analizzare.
     * @return Color Il colore decodificato dalla carta.
     */
    inline Color GetColor(CompactCard c) {
        return static_cast<Color>((c >> 24) & 0xFF);
    }

    /**
     * @brief Estrae il valore o l'azione da una carta compatta (Secondi 8 bit).
     * @param c La carta compatta da analizzare.
     * @return Value Il valore decodificato dalla carta.
     */
    inline Value GetValue(CompactCard c) {
        return static_cast<Value>((c >> 16) & 0xFF);
    }

    /**
     * @brief Estrae l'identificativo univoco a 16-bit della carta (Ultimi 16 bit).
     * @param c La carta compatta da analizzare.
     * @return uint16_t L'ID univoco.
     */
    inline uint16_t GetId(CompactCard c) {
        return static_cast<uint16_t>(c & 0xFFFF);
    }

    /**
     * @brief Crea una nuova CompactCard assemblando colore, valore e ID.
     * @param c Il colore della carta.
     * @param v Il valore della carta.
     * @param id L'identificativo univoco.
     * @return CompactCard La carta impacchettata in un intero a 32-bit.
     */
    inline CompactCard MakeCard(Color c, Value v, uint16_t id) {
        return (static_cast<CompactCard>(c) << 24) | 
               (static_cast<CompactCard>(v) << 16) | 
               (static_cast<CompactCard>(id) & 0xFFFF);
    }

}  // namespace game
