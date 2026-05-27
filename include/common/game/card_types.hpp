#pragma once

#include <cstdint>

namespace game {

    // Lightweight 16-bit card.
    // [ 4 bits: Color ] [ 4 bits: Value ] [ 8 bits: Unique ID ]
    using CompactCard = uint16_t;
    
    enum class Color : uint8_t { 
        kRed = 0, kBlue = 1, kGreen = 2, kYellow = 3, kWild = 4 
    };
    
    enum class Value : uint8_t { 
        k0 = 0, k1, k2, k3, k4, k5, k6, k7, k8, k9, 
        kSkip = 10, kReverse = 11, kDraw2 = 12, kWild = 13, kWildDraw4 = 14 
    };
    
    // Bitwise extractors
    inline Color GetColor(CompactCard c) { return static_cast<Color>((c >> 12) & 0xF); }
    inline Value GetValue(CompactCard c) { return static_cast<Value>((c >> 8) & 0xF); }
    inline uint8_t GetId(CompactCard c)  { return static_cast<uint8_t>(c & 0xFF); }
    
    // Bitwise constructor
    inline CompactCard MakeCard(Color c, Value v, uint8_t id) {
        return (static_cast<uint16_t>(c) << 12) | (static_cast<uint16_t>(v) << 8) | id;
    }

}  
