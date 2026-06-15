#pragma once

#include <cstdint>

/**
 * @file include/common/game/card_types.hpp
 * @brief Defines the fundamental data structures and types for representing the cards in the game.
 *
 * @tag FEAT-CG-001 Compact Card Representation: Use of a 32-bit integer format (CompactCard) to simultaneously store the colour, value and unique ID of the card.
 */

namespace game {

    /**
     * @brief Definition of the CompactCard type.
     * Packs the information of a card within a 32-bit integer.
     * Binary format: `ccccccccvvvvvvvvuuuuuuuuuuuuuuuu`
     * - c (8 bits): Colour
     * - v (8 bits): Value
     * - u (16 bits): Unique card ID
     */
    using CompactCard = uint32_t;

    /**
     * @enum Color
     * @brief Enumerates the possible colours a card can take.
     */
    enum class Color : uint8_t {
        kRed = 0,    /**< Red colour */
        kBlue = 1,   /**< Blue colour */
        kGreen = 2,  /**< Green colour */
        kYellow = 3, /**< Yellow colour */
        kWild = 4    /**< Wild colour (no base colour) */
    };

    /**
     * @enum Value
     * @brief Enumerates the possible values or actions associated with a card.
     */
    enum class Value : uint8_t {
        k0 = 0, k1, k2, k3, k4, k5, k6, k7, k8, k9, /**< Standard numeric values from 0 to 9 */
        kSkip = 10,       /**< Skip card */
        kReverse = 11,    /**< Reverse card */
        kDraw2 = 12,      /**< Draw Two card */
        kWild = 13,       /**< Wild Colour Change card */
        kWildDraw4 = 14   /**< Wild Draw Four card */
    };

    /**
     * @brief Extracts the colour from a compact card (first 8 bits).
     * @param c The compact card to analyze.
     * @return Color The colour decoded from the card.
     */
    inline Color GetColor(CompactCard c) {
        return static_cast<Color>((c >> 24) & 0xFF);
    }

    /**
     * @brief Extracts the value or action from a compact card (second 8 bits).
     * @param c The compact card to analyze.
     * @return Value The value decoded from the card.
     */
    inline Value GetValue(CompactCard c) {
        return static_cast<Value>((c >> 16) & 0xFF);
    }

    /**
     * @brief Extracts the unique 16-bit identifier of the card (last 16 bits).
     * @param c The compact card to analyze.
     * @return uint16_t The unique ID.
     */
    inline uint16_t GetId(CompactCard c) {
        return static_cast<uint16_t>(c & 0xFFFF);
    }

    /**
     * @brief Creates a new CompactCard by assembling colour, value and ID.
     * @param c The colour of the card.
     * @param v The value of the card.
     * @param id The unique identifier.
     * @return CompactCard The card packed into a 32-bit integer.
     */
    inline CompactCard MakeCard(Color c, Value v, uint16_t id) {
        return (static_cast<CompactCard>(c) << 24) |
               (static_cast<CompactCard>(v) << 16) |
               (static_cast<CompactCard>(id) & 0xFFFF);
    }

}  // namespace game
