#pragma once

#include <webserver.hpp>
#include <controllers/lobby_controller.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <functional>
#include <cstdint>

class GameController {
public:
    GameController(WebServer& server, LobbyController& lobby_controller);

private:
    ActionRouter& action_router_;
    LobbyController& lobby_controller_;

    // Maps a Lobby ID to its currently active libuv timer pointer for AFK protection
    std::unordered_map<uint32_t, struct us_timer_t*> active_turn_timers_;

    // --- WebSocket Event Handlers ---
    void HandlePlayCard(WsContext context, const nlohmann::json& message);
    void HandleDrawCard(WsContext context, const nlohmann::json& message);
    void HandleProvideInput(WsContext context, const nlohmann::json& message);
    void HandleCallUno(WsContext context, const nlohmann::json& message);
    
    // --- Core Game Flow ---
    void OnTurnStarted(Lobby* active_lobby);
    void BroadcastGameState(Lobby* current_lobby);

    // --- Timer Management Helpers ---
    void SetTurnTimer(uint32_t lobby_id, int timeout_ms, std::function<void()> callback);
    void ClearTurnTimer(uint32_t lobby_id);
};
