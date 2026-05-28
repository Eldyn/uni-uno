#include <game/rules/standard.hpp>
#include <common/game/effect.hpp>

namespace game {

    void StandardRule::ValidatePlay(GameState* state, CardPlayedEvent& event) {
        Color played_color = GetColor(event.played_card);
        Value played_value = GetValue(event.played_card);
    
        if (played_color == Color::kWild) {
            return; // It remains valid. We are done!
        }
    
        // NOTE: We check active_color since the top card might be a Wild.
        //       since its inherent color is kWild and would return false,
        //       we use the active_color which is whatever the player chose.
        if (played_color == state->active_color) {
            return; 
        }
    
        if (!state->discard_pile.empty()) {
            CompactCard top_card = state->discard_pile.back();
            Value top_value = GetValue(top_card);
    
            if (played_value == top_value) {
                return; 
            }
        }
    
        event.is_valid_play = false;
    }
    
    void StandardRule::OnCardPlayed(GameState* state, CardPlayedEvent& event) {}

}
