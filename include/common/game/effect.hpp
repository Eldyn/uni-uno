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

    class Effect {
    public:
        virtual ~Effect() = default;
        
        // Effects now have full authority to command the MatchInstance
        virtual EffectResult Resolve(GameState* state, MatchInstance* match) = 0;
    };

}
