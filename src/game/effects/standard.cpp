#include "game/game_state.hpp"
#include <game/effects/standard.hpp>
#include <algorithm>
#include <random>

namespace game {

EffectResult DrawEffect::Resolve(GameState* game_state) {
    auto player_iterator = std::ranges::find(game_state->players, target_username_, &Player::username);
    
    if (player_iterator != game_state->players.end()) {
        for (int draw_index = 0; draw_index < count_; ++draw_index) {
            if (game_state->draw_pile.empty()) {
                ReshuffleDiscardIntoDraw(game_state);
                if (game_state->draw_pile.empty()) break; 
            }
            
            player_iterator->hand.push_back(game_state->draw_pile.back());
            game_state->draw_pile.pop_back();
        }
        player_iterator->has_called_uno = false; 
    }
    
    return {EffectStatus::kResolved, "", ""};
}

EffectResult SkipEffect::Resolve(GameState* game_state) {
    game_state->current_player_index += game_state->play_direction;
    int total_players = static_cast<int>(game_state->players.size());
    
    // Wrap around the bounds safely
    if (game_state->current_player_index >= total_players) {
        game_state->current_player_index %= total_players;
    } else {
        while (game_state->current_player_index < 0) {
            game_state->current_player_index += total_players;
        }
    }
    
    return {EffectStatus::kResolved, "", ""};
}

EffectResult ReverseEffect::Resolve(GameState* game_state) {
    game_state->play_direction *= -1;
    
    // In a 2-player game, Reverse acts mathematically identical to a Skip
    if (game_state->players.size() == 2) {
        game_state->effect_queue.push_front(std::make_unique<SkipEffect>());
    }
    
    return {EffectStatus::kResolved, "", ""};
}

EffectResult ChooseColorEffect::Resolve(GameState* game_state) {
    if (!game_state->provided_input.empty()) {
        if (game_state->provided_input == "RED") {
            game_state->active_color = Color::kRed;
        } else if (game_state->provided_input == "BLUE") {
            game_state->active_color = Color::kBlue;
        } else if (game_state->provided_input == "GREEN") {
            game_state->active_color = Color::kGreen;
        } else if (game_state->provided_input == "YELLOW") {
            game_state->active_color = Color::kYellow;
        }
        
        game_state->provided_input.clear();
        return {EffectStatus::kResolved, "", ""};
    }
    
    // Yield the thread and ask the specific player for a color
    return {EffectStatus::kNeedsInput, "choose_color", target_username_};
}

}  // namespace game
