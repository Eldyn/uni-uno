#include "controllers/game_controller.hpp"
#include "common/ws.hpp"
#include "logger.hpp"

using json = nlohmann::json;

struct TurnTimerData {
    std::function<void()> callback;
    uint32_t lobby_id;
    GameController* controller;
};

GameController::GameController(WebServer& server, LobbyController& lobby_controller)
    : action_router_(server.GetActionRouter()), lobby_controller_(lobby_controller) {

    action_router_.On("game_play_card", [this](WsContext context, const json& message) {
        HandlePlayCard(context, message);
        return true;
    });

    action_router_.On("game_draw_card", [this](WsContext context, const json& message) {
        HandleDrawCard(context, message);
        return true;
    });

    action_router_.On("game_submit_input", [this](WsContext context, const json& message) {
        HandleProvideInput(context, message);
        return true;
    });

    action_router_.On("game_call_uno", [this](WsContext context, const json& message) {
        HandleCallUno(context, message);
        return true;
    });

    Logger::Info("[Game] GameController registered");
}

void GameController::HandlePlayCard(WsContext context, const json& message) {
    Lobby* active_lobby = lobby_controller_.GetLobbyByCode(context.socket_data->lobby_code);
    if (!active_lobby || !active_lobby->match) {
        return;
    }

    std::string request_id = ws::GetOr<std::string>(message, "request_id", "");
    uint16_t card_identifier = ws::GetOr<uint16_t>(message, "card_id", 0);

    bool was_play_successful = active_lobby->match->PlayCard(context.socket_data->username, card_identifier);
    
    if (!was_play_successful) {
        ws::SendError(context.socket, context.op_code, "Invalid move", request_id);
        return;
    }

    // 1. Resolve any synchronous effects resulting from the card
    active_lobby->match->Tick();
    
    // 2. The human player successfully acted, so we cancel their AFK timer
    ClearTurnTimer(active_lobby->id); 
    
    // 3. Broadcast the new board state
    BroadcastGameState(active_lobby);
    
    // 4. Trigger evaluation for the next player (in case it is a bot's turn)
    OnTurnStarted(active_lobby);
}

void GameController::HandleDrawCard(WsContext context, const json& message) {
    Lobby* active_lobby = lobby_controller_.GetLobbyByCode(context.socket_data->lobby_code);
    if (!active_lobby || !active_lobby->match) {
        return;
    }

    std::string request_id = ws::GetOr<std::string>(message, "request_id", "");

    bool was_draw_successful = active_lobby->match->DrawCard(context.socket_data->username);
    
    if (!was_draw_successful) {
        ws::SendError(context.socket, context.op_code, "Cannot draw card", request_id);
        return;
    }

    ClearTurnTimer(active_lobby->id);
    BroadcastGameState(active_lobby);
    OnTurnStarted(active_lobby);
}

void GameController::HandleProvideInput(WsContext context, const json& message) {
    Lobby* active_lobby = lobby_controller_.GetLobbyByCode(context.socket_data->lobby_code);
    if (!active_lobby || !active_lobby->match) {
        return;
    }

    std::string input_value = ws::GetOr<std::string>(message, "value", "");
    
    // Feed the user's choice (e.g., "RED") back into the engine
    active_lobby->match->ProvideInput(context.socket_data->username, input_value);
    
    // Resume the execution stack now that the engine has the required input
    active_lobby->match->Tick();
    
    ClearTurnTimer(active_lobby->id);
    BroadcastGameState(active_lobby);
    OnTurnStarted(active_lobby);
}

void GameController::HandleCallUno(WsContext context, const json& message) {
    Lobby* active_lobby = lobby_controller_.GetLobbyByCode(context.socket_data->lobby_code);
    if (!active_lobby || !active_lobby->match) {
        return;
    }

    active_lobby->match->CallUno(context.socket_data->username);
    BroadcastGameState(active_lobby);
}

void GameController::OnTurnStarted(Lobby* active_lobby) {
    if (!active_lobby || !active_lobby->match || active_lobby->match->IsGameOver()) {
        return;
    }

    std::string current_player_username = active_lobby->match->GetCurrentPlayerUsername();
    
    bool is_player_connected = false;
    for (const auto& lobby_member : active_lobby->members) {
        if (lobby_member.username == current_player_username) {
            is_player_connected = lobby_member.is_connected;
            break;
        }
    }

    if (active_lobby->settings.bot_mode == BotTakeoverMode::kPlayInstantly && !is_player_connected) {
        Logger::Info("[Bot] Taking instant turn for disconnected player: ", current_player_username);
        
        active_lobby->match->TakeBotTurn();
        BroadcastGameState(active_lobby);
        
        // Recursively evaluate the next turn (in case multiple players are disconnected)
        OnTurnStarted(active_lobby); 
        return;
    }

    if (active_lobby->settings.bot_mode == BotTakeoverMode::kWaitUntilTurnEnd) {
        uint32_t current_lobby_id = active_lobby->id;
        
        SetTurnTimer(current_lobby_id, active_lobby->settings.turn_time_limit_ms, [this, current_lobby_id, current_player_username]() {
            // NOTE: This lambda executes asynchronously in the future.
            //       We must re-fetch the lobby to ensure it wasn't deleted while we waited.
            Lobby* verified_lobby = lobby_controller_.GetLobbyById(current_lobby_id);
            
            if (verified_lobby && verified_lobby->match) {
                // INFO: Ensure it is still the exact same player's turn (they didn't play at the last millisecond)
                if (verified_lobby->match->GetCurrentPlayerUsername() == current_player_username) {
                    Logger::Info("[Bot] Time expired. Bot playing for AFK player: ", current_player_username);
                    
                    verified_lobby->match->TakeBotTurn();
                    BroadcastGameState(verified_lobby);
                    OnTurnStarted(verified_lobby);
                }
            }
        });
    }
}

void GameController::BroadcastGameState(Lobby* current_lobby) {
    bool is_waiting_for_input = current_lobby->match->IsWaitingForInput();
    std::string pending_player_username = current_lobby->match->GetPendingPlayer();
    std::string required_input_type = current_lobby->match->GetPendingInputType();

    for (const auto& lobby_member : current_lobby->members) {
        // Skip disconnected sockets to prevent write errors
        if (!lobby_member.is_connected || !lobby_member.socket) {
            continue;
        }

        json response_payload = ws::MakeResponse(ws::ServerAction::kGameStateUpdated);
        
        // INFO: Each player receives a serialized state tailored to them (hiding opponents' cards)
        response_payload["game_state"] = current_lobby->match->SerializePlayerState(lobby_member.username);

        // INFO: If the engine yielded and needs input from THIS specific user, attach the prompt
        if (is_waiting_for_input && lobby_member.username == pending_player_username) {
            response_payload["action_required"] = required_input_type;
        }

        lobby_member.socket->send(response_payload.dump(), uWS::OpCode::TEXT);
    }
}

void GameController::SetTurnTimer(uint32_t lobby_id, int timeout_ms, std::function<void()> callback) {
    ClearTurnTimer(lobby_id);

    // INFO: Grab the uWebSockets main event loop
    struct us_loop_t* loop = (struct us_loop_t*) uWS::Loop::get();

    // INFO: Create the timer and reserve enough memory space for our TurnTimerData struct
    struct us_timer_t* timer = us_create_timer(loop, 0, sizeof(TurnTimerData));

    // INFO: Use "placement new" to properly construct the C++ std::function inside the allocated C memory
    TurnTimerData* timer_data = (TurnTimerData*) us_timer_ext(timer);
    new (timer_data) TurnTimerData{std::move(callback), lobby_id, this};

    // INFO: Start the timer (timeout_ms delay, 0 repeating interval for a one-shot timer)
    us_timer_set(timer, [](struct us_timer_t* fired_timer) {
        
        TurnTimerData* data = (TurnTimerData*) us_timer_ext(fired_timer);
        
        if (data->callback) {
            data->callback();
        }

        data->controller->ClearTurnTimer(data->lobby_id);

    }, timeout_ms, 0);

    // INFO: Track it so we can cancel it if the player clicks a card
    active_turn_timers_[lobby_id] = timer;
}

void GameController::ClearTurnTimer(uint32_t lobby_id) {
    auto timer_iterator = active_turn_timers_.find(lobby_id);
    
    if (timer_iterator != active_turn_timers_.end()) {
        struct us_timer_t* timer = timer_iterator->second;
        
        // INFO: Manually call the destructor for the std::function we built with placement new
        TurnTimerData* timer_data = (TurnTimerData*) us_timer_ext(timer);
        timer_data->~TurnTimerData();
        
        // INFO: Tell uSockets to free the timer memory
        us_timer_close(timer);
        
        // INFO: Remove it from our tracking map
        active_turn_timers_.erase(timer_iterator);
    }
}
