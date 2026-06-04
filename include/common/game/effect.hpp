#pragma once

#include <string>

namespace game {

    struct GameState;
    class MatchInstance;

    enum class EffectStatus { kResolved, kNeedsInput, kError };
    
    struct EffectResult {
        EffectStatus status;
        std::string input_type;
        std::string target_player;
        std::string input_context = ""; // Generalized JSON payload for the frontend
    };

    enum class EffectType {
        kAdvanceTurn, kDraw, kSkip, kPlayCard, kDecideDrawnCard, kChooseColor, kReverse, kCustom
    };

    class Effect {
    public:
        virtual ~Effect() = default;
        
        virtual EffectType GetType() const { return EffectType::kCustom; }
        virtual EffectResult Resolve(GameState* state, MatchInstance* match) = 0;
    };

}
