#pragma once

#include <common/game/effect.hpp>
#include <string>

namespace game {
    class DecideSwapTargetEffect : public Effect {
    public:
        explicit DecideSwapTargetEffect(std::string username) : username_(std::move(username)) {}

        EffectResult Resolve(GameState* state, MatchInstance* match) override;
        EffectType GetType() const override { return EffectType::kDecideSwapTarget; }
    private:
        std::string username_;
    };
}
