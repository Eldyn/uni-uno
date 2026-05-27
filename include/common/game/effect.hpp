#pragma once

#include <game/game_state.hpp>

namespace game {

    class Effect {
    public:
        virtual ~Effect() = default;
        virtual EffectResult Resolve(GameState* state) = 0;
    };
}
