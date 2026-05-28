#pragma once

#include <common/game/gamerule.hpp>
#include <controllers/lobby_controller.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

struct LobbySettings;

namespace game {
    class MatchInstance {
    public:
        explicit MatchInstance(const std::vector<std::string>& usernames, LobbySettings& settings);
        
        void Start();
        void Tick(); 
        
        // Core Actions
        bool PlayCard(const std::string& username, uint16_t card_id);
        bool DrawCard(const std::string& username);
        void ProvideInput(const std::string& username, const std::string& input);
        
        void CallUno(const std::string& username);
    
        // Bot Integration
        bool TakeBotTurn();
        std::string GetCurrentPlayerUsername() const;
    
        // State Queries
        bool IsWaitingForInput() const { return !state_.pending_player.empty(); }
        std::string GetPendingPlayer() const { return state_.pending_player; }
        std::string GetPendingInputType() const { return state_.pending_input_type; }
        
        bool IsGameOver() const { return state_.status == MatchStatus::kFinished; }
        std::string GetWinner() const { return state_.winner; }
    
        nlohmann::json SerializePlayerState(const std::string& username) const;
    
    private:
        GameState state_;
        LobbySettings& settings_;
        std::vector<std::unique_ptr<GameRule>> active_rules_;
    
        void AdvanceTurn();
        void GenerateDeck();
        Player* GetPlayer(const std::string& username);
    };
}
