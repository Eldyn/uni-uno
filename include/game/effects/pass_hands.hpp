#include <common/game/effect.hpp>

namespace game {
    class PassHandsEffect : public Effect {
    public:
        EffectResult Resolve(GameState* state, MatchInstance* match) override;
    };
}
