#pragma once

#include <game/game_state.hpp>
#include <common/game/gamerule.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace game {
    
    class MatchInstance {
    public:
        explicit MatchInstance(const std::vector<std::string>& usernames);
        
        void Start();
        void Tick(); 
        
        bool PlayCard(const std::string& username, uint8_t card_id);
        void ProvideInput(const std::string& username, const std::string& input);
    
        bool IsWaitingForInput() const { return !state_.pending_player.empty(); }
        std::string GetPendingPlayer() const { return state_.pending_player; }
        std::string GetPendingInputType() const { return state_.pending_input_type; }
        
        nlohmann::json GetStateForPlayer(const std::string& username) const;
    
    private:
        GameState state_;
        std::vector<std::unique_ptr<GameRule>> active_rules_;
    
        void AdvanceTurn();
        void GenerateClassicalDeck();
    };
    
}
