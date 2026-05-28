#pragma once

#include <common/game/gamerule.hpp>

namespace game {
   class StandardRule : public GameRule {
       public:
        ~StandardRule() override = default;
        void ValidatePlay(GameState* state, CardPlayedEvent& event) override;
        void OnCardPlayed(GameState* state, CardPlayedEvent& event) override;
   };
}
