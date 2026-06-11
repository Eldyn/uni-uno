#include <common/game/gamerule.hpp>
#include <common/game/effect.hpp>
#include <game/game_state.hpp>
#include <game/match_instance.hpp>
#include <game/rule_registry.hpp>
#include <game/effects/pass_hands.hpp>
#include <game/effects/decide_swap_target.hpp>
#include <nlohmann/json.hpp>

namespace game {
    class SevenZeroRule : public GameRule {
    public:
        ~SevenZeroRule() override = default;

        void ValidatePlay(GameState* state, CardPlayedEvent& event) override {}

        void OnCardPlayed(GameState* state, CardPlayedEvent& event) override {
            Value card_value = GetValue(event.played_card);

            if (card_value == Value::k0) state->effect_queue.push_back(std::make_unique<PassHandsEffect>());
            else if (card_value == Value::k7) state->effect_queue.push_back(std::make_unique<DecideSwapTargetEffect>(event.player_username));
        }
    };

   static RuleRegistrar registrar("seven_zero", []() { 
        return std::make_unique<SevenZeroRule>(); 
    });
}
