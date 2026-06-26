#include <common/game/gamerule.hpp>
#include <game/game_state.hpp>
#include <game/rule_registry.hpp>
#include <algorithm>

namespace game {
    class NoBluffingRule : public GameRule {
    public:
        void ValidatePlay(const GameState* state, CardPlayedEvent& event) override {
            if (event.is_out_of_turn) return;

            if (GetValue(event.played_card) == Value::kJollyDraw4) {
                auto player_it = std::ranges::find(state->players, event.player_username, &Player::username);
                
                if (player_it != state->players.end()) {
                    bool has_matching_type = false;

                    for (CompactCard card : player_it->hand) {
                        if (GetType(card) == state->active_type) {
                            has_matching_type = true;
                            break;
                        }
                    }
                    
                    if (has_matching_type) {
                        event.is_valid_play = false;
                        event.is_handled = true; // Stop StandardRule from validating this
                    }
                }
            }
        }
    };

    static RuleRegistrar registrar("no_bluffing", []() {
        return std::make_unique<NoBluffingRule>();
    }, "No Bluffing", "+4 can only be played if you have no card matching the current color.");
}
