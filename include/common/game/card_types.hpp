#pragma once

#include <cstdint>

/**
 * @file include/common/game/card_types.hpp
 * @brief Defines the fundamental data structures and types for representing the cards in the game.
 *
 * @tag FEAT-CG-001 Compact Card Representation: Use of a 32-bit integer format (CompactCard) to simultaneously store the type, value and unique ID of the card.
 */

namespace game {

    /**
     * @brief Definition of the CompactCard type.
     * Packs the information of a card within a 32-bit integer.
     * Binary format: `ttttttttvvvvvvvvuuuuuuuuuuuuuuuu`
     * - t (8 bits): Type
     * - v (8 bits): Value
     * - u (16 bits): Unique card ID
     */
    using CompactCard = uint32_t;

    /**
     * @enum Type
     * @brief Enumerates the possible types a card can take.
     */
    enum class Type : uint8_t {
        kRed    = 0, /**< Red type */
        kBlue   = 1, /**< Blue type */
        kGreen  = 2, /**< Green type */
        kYellow = 3, /**< Yellow type */
        kWhite  = 4  /**< White type (no base type) */
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
        kJolly = 13,      /**< Jolly Type Change card */
        kJollyDraw4 = 14  /**< Jolly Draw Four card */
    };

    /**
     * @brief Extracts the type from a compact card (first 8 bits).
     * @param c The compact card to analyze.
     * @return Type The type decoded from the card.
     */
    inline Type GetType(CompactCard c) {
        return static_cast<Type>((c >> 24) & 0xFF);
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
     * @brief Creates a new CompactCard by assembling type, value and ID.
     * @param t The type of the card.
     * @param v The value of the card.
     * @param id The unique identifier.
     * @return CompactCard The card packed into a 32-bit integer.
     */
    inline CompactCard MakeCard(Type t, Value v, uint16_t id) {
        return (static_cast<CompactCard>(t) << 24) |
               (static_cast<CompactCard>(v) << 16) |
               (static_cast<CompactCard>(id) & 0xFFFF);
    }

}  // namespace game
