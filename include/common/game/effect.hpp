#pragma once

#include <game/game_state.hpp>
#include <string>

namespace game {

    struct GameState;

    enum class EffectStatus { kResolved, kNeedsInput, kError };
    
    struct EffectResult {
        EffectStatus status;
        std::string input_type;
        std::string target_player;
    };

    class Effect {
    public:
        virtual ~Effect() = default;
        virtual EffectResult Resolve(GameState* state) = 0;
    };
}
