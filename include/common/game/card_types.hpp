#pragma once

#include <cstdint>

namespace game {

    // INFO: We pack the card in 32 bits (much more compact than strings)
    //       If we ever have time for optimisations, it would be fun to try 
    //       dynamic bit size based on the lobby settings.
    //       Format: ccccccccvvvvvvvvuuuuuuuuuuuuuuuu
    using CompactCard = uint32_t;

    enum class Color : uint8_t { 
        kRed = 0, kBlue = 1, kGreen = 2, kYellow = 3, kWild = 4 
    };

    enum class Value : uint8_t { 
        k0 = 0, k1, k2, k3, k4, k5, k6, k7, k8, k9, 
        kSkip = 10, kReverse = 11, kDraw2 = 12, kWild = 13, kWildDraw4 = 14 
    };

    inline Color GetColor(CompactCard c) { return static_cast<Color>((c >> 24) & 0xFF); }
    inline Value GetValue(CompactCard c) { return static_cast<Value>((c >> 16) & 0xFF); }
    inline uint16_t GetId(CompactCard c) { return static_cast<uint16_t>(c & 0xFFFF); } 

    inline CompactCard MakeCard(Color c, Value v, uint16_t id) {
        return (static_cast<uint32_t>(c) << 24) | 
            (static_cast<uint32_t>(v) << 16) | 
            static_cast<uint32_t>(id);
    }

}  // namespace game
