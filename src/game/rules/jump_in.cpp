#include <common/game/gamerule.hpp>
#include <game/game_state.hpp>
#include <game/rule_registry.hpp>

namespace game {
    class JumpInRule : public GameRule {
    public:
        void ValidatePlay(GameState* state, CardPlayedEvent& event) override {
            if (event.is_out_of_turn) {
                if (state->discard_pile.empty()) return;

                CompactCard top_card = state->discard_pile.back();

                if (GetColor(event.played_card) == GetColor(top_card) && 
                    GetValue(event.played_card) == GetValue(top_card)) {

                    event.is_valid_play = true;
                    event.is_handled = true;
                }
            }
        }

        void OnCardPlayed(GameState* state, CardPlayedEvent& event) override {
            if (event.is_out_of_turn) {
                auto it = std::ranges::find(state->players, event.player_username, &Player::username);
                if (it != state->players.end()) {
                    state->current_player_index = std::distance(state->players.begin(), it);
                    Logger::Info("[Match] ", event.player_username, " successfully JUMPED IN!");
                }
            }
        }
    };

    static RuleRegistrar registrar("jump_in", []() {
        return std::make_unique<JumpInRule>();
    }, "Jump In", "Play an identical card (same number and color) out of turn to steal the turn.");
}
