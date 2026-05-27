#pragma once

#include <common/game/card_types.hpp>
#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace game {
    
    class Effect; // Forward declaration to break circular dependency
    
    struct Player {
        std::string username;
        std::vector<CompactCard> hand;
        bool has_called_uno = false;
    };
    
    // Pure data container. The single source of truth.
    struct GameState {
        std::vector<Player> players;
        int current_player_index = 0;
        int play_direction = 1; 
        
        std::vector<CompactCard> draw_pile;
        std::vector<CompactCard> discard_pile;
        Color active_color = Color::kRed; 
    
        // The Effect Stack
        std::deque<std::unique_ptr<Effect>> effect_queue;
    
        // Async Input Blockers
        std::string pending_player;
        std::string pending_input_type;
        std::string provided_input; 
    };
    
    enum class EffectStatus { kResolved, kNeedsInput, kError };
    
    struct EffectResult {
        EffectStatus status;
        std::string input_type;
        std::string target_player;
    };
    
}  
