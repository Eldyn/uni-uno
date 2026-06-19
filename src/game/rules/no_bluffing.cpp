#include <common/game/gamerule.hpp>
#include <game/game_state.hpp>
#include <game/rule_registry.hpp>
#include <algorithm>

namespace game {
    class NoBluffingRule : public GameRule {
    public:
        void ValidatePlay(const GameState* state, CardPlayedEvent& event) override {
            if (event.is_out_of_turn) return;

            if (GetValue(event.played_card) == Value::kWildDraw4) {
                auto player_it = std::ranges::find(state->players, event.player_username, &Player::username);
                
                if (player_it != state->players.end()) {
                    bool has_matching_color = false;
                    
                    for (CompactCard card : player_it->hand) {
                        if (GetColor(card) == state->active_color) {
                            has_matching_color = true;
                            break;
                        }
                    }
                    
                    if (has_matching_color) {
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
