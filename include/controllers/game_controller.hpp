#pragma once
#include "webserver.hpp"
#include "lobby_controller.hpp"
#include <nlohmann/json.hpp>

class GameController {
public:
    GameController(WebServer& server, LobbyController& lobby_controller);

private:
    ActionRouter& action_router_;
    LobbyController& lobby_controller_;

    void HandlePlayCard(WsContext ctx, const nlohmann::json& msg);
    void HandleProvideInput(WsContext ctx, const nlohmann::json& msg);
    
    // Helper to push state to everyone in the match
    void BroadcastGameState(Lobby* lobby);
};
