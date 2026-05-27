#pragma once

#include <common/game/card_types.hpp>
#include <game/game_state.hpp>

namespace game {
    
    struct CardPlayedEvent {
        std::string player_username;
        CompactCard played_card;
        bool is_valid_play = true; 
    };
    
    class GameRule {
    public:
        virtual ~GameRule() = default;
    
        // Called before the card is committed. Set event.is_valid_play to false to reject.
        virtual void ValidatePlay(GameState* state, CardPlayedEvent& event) {}
        
        // Called after the card is successfully played to push Effects to the queue.
        virtual void OnCardPlayed(GameState* state, CardPlayedEvent& event) {}
    };
}
