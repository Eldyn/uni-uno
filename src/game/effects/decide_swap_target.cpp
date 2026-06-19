#include <nlohmann/json.hpp>
#include <common/game/gamerule.hpp>
#include <common/game/effect.hpp>
#include <game/game_state.hpp>
#include <game/match_instance.hpp>
#include <game/effects/decide_swap_target.hpp>
#include <game/effects/pass_hands.hpp>

namespace game {
    EffectResult DecideSwapTargetEffect::Resolve(GameState* state, MatchInstance* match) {
        if (!state->provided_input.empty()) {
            std::string target_username = state->provided_input;
            state->provided_input.clear();

            Logger::Info("[Effect] 7-Swap received target: '", target_username, "' from player: '", username_, "'");

            auto p1 = std::ranges::find(state->players, username_, &Player::username);
            auto p2 = std::ranges::find(state->players, target_username, &Player::username);

            if (p1 != state->players.end() && p2 != state->players.end() && p1 != p2) {
                Logger::Info("[Effect] Executing swap between ", p1->username, " and ", p2->username);
                std::swap(p1->hand, p2->hand);
                std::swap(p1->has_called_uno, p2->has_called_uno);

                return {EffectStatus::kResolved};
            }

            Logger::Warn("Invalid target for 7 swap: ", target_username);
        }

        nlohmann::json action_context = nlohmann::json::array();
        for (const auto& p : state->players) {
            if (p.username != username_) {
                action_context.push_back(p.username);
            }
        }

        return {EffectStatus::kNeedsInput, Action::kChooseTarget, username_, action_context.dump()};
    };
}
