#include "webserver.hpp"
#include "websocket_context.hpp"
#include <WebSocketProtocol.h>
#include <controllers/lobby_controller.hpp>
#include <common/ws.hpp>
#include <logger.hpp>
#include <openssl/rand.h>
#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <string>

using namespace std::chrono;
using std::string, std::vector, std::runtime_error, std::erase_if, std::memory_order_relaxed, std::transform, std::to_string; 

//  Invite code alphabet — A-Z + 0-9 = 36 chars.
//  6 characters → 36^6 = ~2.18 billion combinations.
//  At one guess per second with no rate limiting that's 69 years to exhaust.
//  In practice lobbies live for ~30 minutes, making exhaustion impossible.
static constexpr char kCodeAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static constexpr int  kCodeLen        = 6;
static constexpr int  kAlphabetLen    = 36;   

LobbyController::LobbyController(WebServer& server) : action_router_(server.GetActionRouter()) {
    action_router_.On("lobby_create", [this](WsContext ctx, const json& msg) {
        HandleCreate(ctx, msg);
        return true;
    });

    action_router_.On("lobby_join", [this](WsContext ctx, const json& msg) {
        HandleJoin(ctx, msg);
        return true;
    });

    action_router_.On("lobby_rejoin", [this](WsContext ctx, const json& msg) {
        HandleRejoin(ctx, msg);
        return true;
    });

    action_router_.On("lobby_leave", [this](WsContext ctx, const json& msg) {
        HandleLeave(ctx, msg);
        return true;
    });

    action_router_.On("lobby_list", [this](WsContext ctx, const json& msg) {
        HandleList(ctx, msg);
        return true;
    });

    action_router_.On("lobby_kick", [this](WsContext ctx, const json& msg) {
        HandleKick(ctx, msg);
        return true;
    });

    action_router_.On("lobby_promote", [this](WsContext ctx, const json& msg) {
        HandlePromote(ctx, msg);
        return true;
    });

    action_router_.On("lobby_update_settings", [this](WsContext ctx, const json& msg) {
        HandleUpdateSettings(ctx, msg);
        return true;
    });

    action_router_.On("lobby_start_game", [this](WsContext context, const nlohmann::json& message) {
        HandleStartGame(context, message);
        return true;
    });

    server.OnConnectionOpen([this](AppWebSocket* ws, PerSocketData* sd) {
        this->OnOpen(ws, sd);
    });

    server.OnConnectionClose([this](AppWebSocket* ws, PerSocketData* sd) {
        this->OnClose(ws, sd);
    });

    eviction_timer_ = us_create_timer((us_loop_t*)uWS::Loop::get(), 0, sizeof(LobbyController*));
    *(LobbyController**)us_timer_ext(eviction_timer_) = this;

    us_timer_set(eviction_timer_, [](us_timer_t* t) {
        auto* self = *(LobbyController**)us_timer_ext(t);
        auto  now  = steady_clock::now();

        vector<uint32_t> to_broadcast;
        vector<uint32_t> to_destroy;

        for (auto& [id, lobby] : self->lobbies_) {
            bool changed = false;

            // NOTE: If we want evicted users to get a clean visual "You were disconnected" 
            //       notification, we could invoke a specialized variant of RemoveMember here.
            //       For now, we clean them silently as they are already offline.
            erase_if(lobby.members, [&](const LobbyMember& m) {
                if (!m.is_connected) {
                    auto elapsed = duration_cast<milliseconds>(now - m.disconnected_at).count();
                    if (elapsed > kReconnectGraceMs) {
                        Logger::Log("[Lobby] Evicted ", m.username, " from lobby ", id, " (grace expired)");
                        changed = true;
                        return true;
                    }
                }
                return false;
            });

            if (changed) {
                if (lobby.members.empty()) {
                    to_destroy.push_back(id);
                } else {
                    to_broadcast.push_back(id);
                }
            }
        }

        for (uint32_t id : to_broadcast) {
            self->BroadcastUpdate(self->lobbies_.at(id));
        }

        for (uint32_t id : to_destroy) {
            const auto& lobby = self->lobbies_.at(id);
            self->code_to_id_.erase(lobby.invite_code);
            self->lobbies_.erase(id);
            Logger::Log("[Lobby] Destroyed empty lobby ", id);
        }

    }, 1000, 1000);

    Logger::Info("[Lobby] Registered — grace window: " + to_string(kReconnectGraceMs / 1000) + "s");
}

LobbyController::~LobbyController() {
    if (eviction_timer_) {
        us_timer_close(eviction_timer_);
        eviction_timer_ = nullptr;
    }
}

void LobbyController::OnOpen(AppWebSocket* ws, PerSocketData* sd) {
    Lobby* lobby = FindLobbyForUser(sd->username);
    if (!lobby) return;

    for (auto& member : lobby->members) {
        if (member.username == sd->username) {
            Logger::Log("[Lobby] Reconnect: ", sd->username, " back in lobby ", lobby->id);

            member.socket    = ws;
            member.is_connected = true;
            member.disconnected_at = steady_clock::time_point{};
            sd->lobby_code = lobby->invite_code;

            BroadcastUpdate(*lobby);
            return;
        }
    }
}

void LobbyController::OnClose(AppWebSocket* ws, PerSocketData* sd) {
    if (sd->lobby_code.empty()) return;

    auto it = code_to_id_.find(sd->lobby_code);
    if (it == code_to_id_.end()) return;

    auto lobby_it = lobbies_.find(it->second);
    if (lobby_it == lobbies_.end()) return;

    Lobby& lobby = lobby_it->second;

    for (auto& member : lobby.members) {
        if (member.username == sd->username) {
            Logger::Log("[Lobby] Disconnect: ", sd->username, " in lobby ", lobby.id, " — grace window open");
            member.is_connected    = false;
            member.socket          = nullptr;
            member.disconnected_at = steady_clock::now();
            BroadcastUpdate(lobby);
            return;
        }
    }
}

void LobbyController::HandleCreate(WsContext ctx, const json& message) {
    const string& username = ctx.socket_data->username;
    const string request_id = ws::GetOr<string>(message, "request_id", "");

    if (!ctx.socket_data->lobby_code.empty()) {
        ws::SendError(ctx.socket, ctx.op_code, "Already in a lobby", request_id);
        return;
    }

    string code;
    int attempts = 0;
    do {
        code = GenerateInviteCode();
        if (++attempts > 10)
            throw runtime_error("[Lobby] Failed to generate unique code");
    } while (code_to_id_.count(code));

    uint32_t id = next_id_.fetch_add(1, memory_order_relaxed);

    Lobby& lobby = lobbies_[id];
    lobby.id          = id;
    lobby.is_public   = ws::GetOr<bool>(message, "is_public", false);
    lobby.invite_code = code;
    lobby.host        = username;
    lobby.name        = ws::GetOr<string>(message, "name", username+"'s lobby");
    lobby.members.emplace_back(username, ctx.socket, true);

    code_to_id_[code] = id;
    ctx.socket_data->lobby_code = code;

    Logger::Log("[Lobby] Created lobby ", id, " code=", code, " host=", username);

    auto resp = MakeResponse(ws::ServerAction::kLobbyJoined, request_id);
    resp["lobby"] = json{
        {"is_public", lobby.is_public},
        {"invite_code", code},
        {"host", lobby.host},
        {"members", MemberListJson(lobby)},
        {"name", lobby.name}
    };
    ctx.socket->send(resp.dump(), ctx.op_code);
}

void LobbyController::HandleJoin(WsContext ctx, const json& message) {
    const string request_id = ws::GetOr<string>(message, "request_id", "");
    auto tryCode = ws::Get<string>(message, "code");

    if (!tryCode) {
        ws::SendError(ctx.socket, ctx.op_code, "Malformed or missing join code", request_id);
        return;
    }

    if (!ctx.socket_data->lobby_code.empty()) {
        ws::SendError(ctx.socket, ctx.op_code, "Already in a lobby", request_id);
        return;
    }

    string code = tryCode.value();
    transform(code.begin(), code.end(), code.begin(), ::toupper);

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) {
        ws::SendError(ctx.socket, ctx.op_code, "Lobby not found", request_id);
        return;
    }

    auto lobby_it = lobbies_.find(it->second);
    if (lobby_it == lobbies_.end()) {
        code_to_id_.erase(it); 
        ws::SendError(ctx.socket, ctx.op_code, "Lobby not found", request_id);
        return;
    }

    Lobby& lobby = lobby_it->second;
    
    if (lobby.members.size() >= 4) {
        ws::SendError(ctx.socket, ctx.op_code, "Lobby full", request_id);
        return;
    }

    const string& username = ctx.socket_data->username;

    if (std::ranges::contains(lobby.members, username, &LobbyMember::username)) {
        ws::SendError(ctx.socket, ctx.op_code, "Already a member", request_id);
        return;
    }

    lobby.members.emplace_back(username, ctx.socket, true);
    ctx.socket_data->lobby_code = code;

    Logger::Log("[Lobby] ", username, " joined lobby ", lobby.id);

    auto resp = MakeResponse(ws::ServerAction::kLobbyJoined, request_id);
    resp["lobby"] = json({
        {"invite_code", code},
        {"host",        lobby.host},
        {"members",     MemberListJson(lobby)},
        {"name",        lobby.name}
    });
    ctx.socket->send(resp.dump(), ctx.op_code);

    BroadcastUpdate(lobby);
}

void LobbyController::HandleRejoin(WsContext ctx, const json& message) {
    const std::string request_id = ws::GetOr<string>(message, "request_id", "");

    if (!message.contains("code") || !message["code"].is_string()) {
        ws::SendError(ctx.socket, ctx.op_code, "Missing lobby code", request_id);
        return;
    }

    string code = message["code"];
    transform(code.begin(), code.end(), code.begin(), ::toupper);

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) {
        auto resp = MakeResponse(ws::ServerAction::kLobbyEvicted, request_id);
        resp["reason"] = "Lobby expired";
        ctx.socket->send(resp.dump(), ctx.op_code);
        return;
    }

    auto lobby_it = lobbies_.find(it->second);
    if (lobby_it == lobbies_.end()) {
        code_to_id_.erase(it); 
        auto resp = MakeResponse(ws::ServerAction::kLobbyEvicted, request_id);
        resp["reason"] = "Lobby expired or invalid";
        ctx.socket->send(resp.dump(), ctx.op_code);
        return;
    }

    Lobby& lobby = lobby_it->second;
    const string& username = ctx.socket_data->username;

    if (std::ranges::contains(lobby.members, username, &LobbyMember::username)) {
        if (lobby.match) {
            // TODO: Also handle Game rejoin
        }

        auto resp = MakeResponse(ws::ServerAction::kLobbyJoined, request_id);
        resp["lobby"] = json({
            {"invite_code", code},
            {"host",        lobby.host},
            {"members",     MemberListJson(lobby)},
            {"name",        lobby.name}
        });
        ctx.socket->send(resp.dump(), ctx.op_code);
        return;
    }

    ws::SendError(ctx.socket, ctx.op_code, "No longer a member of this lobby", request_id);
}

void LobbyController::HandleLeave(WsContext ctx, const json& message) {
    const string& username   = ctx.socket_data->username;
    const string& code       = ctx.socket_data->lobby_code;
    const string  request_id = ws::GetOr<string>(message, "request_id", "");

    if (code.empty()) {
        ws::SendError(ctx.socket, ctx.op_code, "Not in a lobby", request_id);
        return;
    }

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) {
        ws::SendError(ctx.socket, ctx.op_code, "Lobby not found", request_id);
        return;
    }

    Lobby& lobby = lobbies_.at(it->second);
    uint32_t id  = lobby.id;
    bool was_host = (lobby.host == username);

    // Pass true for explicit_action, pass request_id so they match their pending promise
    bool lobby_still_exists = RemoveMember(id, username, true, request_id);

    if (lobby_still_exists && was_host) {
        for (const auto& m : lobbies_.at(id).members) {
            if (m.is_connected) {
                lobbies_.at(id).host = m.username;
                Logger::Log("[Lobby] Host auto-passed to ", m.username, " in lobby ", id);
                break;
            }
        }
        BroadcastUpdate(lobbies_.at(id));
    }
}

void LobbyController::HandleList(WsContext ctx, const json& message) {
    json list = json::array();
    string request_id = ws::GetOr<string>(message, "request_id", "");

    for (const auto& [id, lobby] : lobbies_) {
        bool any_connected = std::ranges::any_of(lobby.members, &LobbyMember::is_connected);

        if (!any_connected) continue;
        if (static_cast<int>(lobby.members.size()) >= kMaxMembers) continue;
       
        if (lobby.is_public) {
            list.push_back({
                {"name", lobby.name},
                {"member_count", lobby.members.size()},
                {"invite_code", lobby.invite_code}
            });
        } else {
            list.push_back({
                {"name", lobby.name},
                {"member_count", lobby.members.size()}
            });
        }
    }

    auto resp = MakeResponse(ws::ServerAction::kLobbyList, request_id);
    resp["lobbies"] = list;
    ctx.socket->send(resp.dump(), ctx.op_code);
}

void LobbyController::HandlePromote(WsContext ctx, const json& message) {
    const string& code       = ctx.socket_data->lobby_code;
    const string& request_id = ws::GetOr<string>(message, "request_id", "");
    auto try_username = ws::Get<string>(message, "username");

    if (!try_username) {
        ws::SendError(ctx.socket, ctx.op_code, "Username not specified", request_id);
        return; 
    }
    
    const string& username = try_username.value();

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) {
        ws::SendError(ctx.socket, ctx.op_code, "Lobby not found", request_id);
        return;
    }

    auto lobby_it = lobbies_.find(it->second);
    if (lobby_it == lobbies_.end()) {
        ws::SendError(ctx.socket, ctx.op_code, "Lobby data integrity error", request_id);
        return;
    }
    Lobby& lobby = lobby_it->second;

    if (ctx.socket_data->username != lobby.host) {
        ws::SendError(ctx.socket, ctx.op_code, "You must be the host of this lobby!", request_id);
        return;
    }

    if (!std::ranges::contains(lobby.members, username, &LobbyMember::username)) {
        ws::SendError(ctx.socket, ctx.op_code, "No such user is in the lobby", request_id);
        return;
    }

    lobby.host = username;
    BroadcastUpdate(lobby);

    // Optional: Send confirmation back to the host explicitly if required by front-end
}

void LobbyController::HandleKick(WsContext ctx, const json& message) {
    const string& code       = ctx.socket_data->lobby_code;
    const string& request_id = ws::GetOr<string>(message, "request_id", "");
    auto try_username = ws::Get<string>(message, "username");

    if (!try_username) {
        ws::SendError(ctx.socket, ctx.op_code, "Username not specified", request_id);
        return; 
    }
    
    const string& username = try_username.value();

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) {
        ws::SendError(ctx.socket, ctx.op_code, "Lobby not found", request_id);
        return;
    }

    auto lobby_it = lobbies_.find(it->second);
    if (lobby_it == lobbies_.end()) {
        ws::SendError(ctx.socket, ctx.op_code, "Lobby data integrity error", request_id);
        return;
    }
    Lobby& lobby = lobby_it->second;

    // 1. Authorization Check
    if (ctx.socket_data->username != lobby.host) {
        ws::SendError(ctx.socket, ctx.op_code, "You must be the host of this lobby!", request_id);
        return;
    }

    // 2. Prevent Host Self-Kick
    if (username == lobby.host) {
        ws::SendError(ctx.socket, ctx.op_code, "You cannot kick yourself", request_id);
        return;
    }

    // 3. Target Verification
    if (!std::ranges::contains(lobby.members, username, &LobbyMember::username)) {
        ws::SendError(ctx.socket, ctx.op_code, "No such user is in the lobby", request_id);
        return;
    }

    // 4. Remove target. RemoveMember will notify the victim socket directly.
    bool lobby_still_exists = RemoveMember(lobby.id, username, false);

    // 5. If the lobby survived, broadcast the update to remaining players
    if (lobby_still_exists) {
        BroadcastUpdate(lobby);
    }
}

void LobbyController::HandleUpdateSettings(WsContext ctx, const json& message) {
    const string& code       = ctx.socket_data->lobby_code;
    const string& request_id = ws::GetOr<string>(message, "request_id", "");

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) {
        ws::SendError(ctx.socket, ctx.op_code, "Lobby not found", request_id);
        return;
    }

    Lobby& lobby = lobbies_.at(it->second);

    if (ctx.socket_data->username != lobby.host) {
        ws::SendError(ctx.socket, ctx.op_code, "Only the host can modify settings", request_id);
        return;
    }

    bool changed = false;

    if (message.contains("is_public") && message["is_public"].is_boolean()) {
        lobby.is_public = message["is_public"];
        changed = true;
    }

    if (message.contains("name") && message["name"].is_string()) {
        string tentative_name = message["name"];
        if (!tentative_name.empty() && tentative_name.length() <= 32) {
            lobby.name = tentative_name;
            changed = true;
        }
    }

    if (changed) {
        BroadcastUpdate(lobby);
    }
}

void LobbyController::HandleStartGame(WsContext context, const nlohmann::json& message) {
    const string& code       = context.socket_data->lobby_code;
    const string& request_id = ws::GetOr<string>(message, "request_id", "");

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) {
        ws::SendError(context.socket, context.op_code, "Lobby not found", request_id);
        return;
    }

    Lobby& lobby = lobbies_.at(it->second);


    if (lobby.host != context.socket_data->username) {
        ws::SendError(context.socket, context.op_code, "Only the host can start the game.", request_id);
        return;
    }

    if (lobby.match != nullptr) {
        ws::SendError(context.socket, context.op_code, "Match already started!", request_id);
        return;
    }

    if (lobby.members.size() < 2) {
        ws::SendError(context.socket, context.op_code, "Not enough players to start.", request_id);
        return;
    }

    std::vector<std::string> player_usernames;
    for (const auto& lobby_member : lobby.members) {
        player_usernames.push_back(lobby_member.username);
    }

    // 4. Instantiate and start the game engine!
    lobby.match = std::make_unique<game::MatchInstance>(player_usernames, lobby.settings);
    lobby.match->Start();

    Logger::Info("[Lobby] Match started by host '", lobby.host, "' in lobby ", lobby.id);

    // 5. Broadcast the initial game state to all players.
    // (This automatically triggers storeGame on the frontend to navigate to the GameScreen)
    for (const auto& lobby_member : lobby.members) {
        if (!lobby_member.is_connected || !lobby_member.socket) {
            continue;
        }

        nlohmann::json response_payload = ws::MakeResponse(ws::ServerAction::kGameStateUpdated);
        
        // Serialize state specifically for this player (hiding opponent's cards)
        response_payload["game_state"] = lobby.match->SerializePlayerState(lobby_member.username);
        
        lobby_member.socket->send(response_payload.dump(), uWS::OpCode::TEXT);
    }
}

string LobbyController::GenerateInviteCode() {
    uint8_t raw[kCodeLen];
    if (RAND_bytes(raw, kCodeLen) != 1)
        throw runtime_error("[Lobby] RAND_bytes failed generating invite code");

    string code(kCodeLen, ' ');
    for (int i = 0; i < kCodeLen; ++i)
        code[i] = kCodeAlphabet[raw[i] % kAlphabetLen];
    return code;
}

json LobbyController::MemberListJson(const Lobby& lobby) {
    json arr = json::array();
    for (const auto& m : lobby.members) {
        arr.push_back({
            {"username",  m.username},
            {"is_connected", m.is_connected},
            {"is_host",   m.username == lobby.host},
        });
    }
    return arr;
}

void LobbyController::BroadcastUpdate(const Lobby& lobby) const {
    auto notification = MakeResponse(ws::ServerAction::kLobbyUpdated);
    notification["lobby"] = json{
        {"is_public", lobby.is_public},
        {"invite_code", lobby.invite_code},
        {"host", lobby.host},
        {"members", MemberListJson(lobby)},
        {"name", lobby.name}
    };

    for (const auto& m : lobby.members) {
        if (m.is_connected && m.socket) {
            m.socket->send(notification.dump(), uWS::OpCode::TEXT);
        }
    }
}

bool LobbyController::RemoveMember(uint32_t lobby_id, const string& username, bool explicit_leave, const string& request_id) {
    auto it = lobbies_.find(lobby_id);
    if (it == lobbies_.end()) return false;

    Lobby& lobby = it->second;

    auto member_it = std::ranges::find(lobby.members, username, &LobbyMember::username);
    if (member_it != lobby.members.end()) {
        if (member_it->is_connected && member_it->socket) {
            PerSocketData* sd = member_it->socket->getUserData();
            if (sd) sd->lobby_code.clear();

            json response;
            if (explicit_leave) {
                response = MakeResponse(ws::ServerAction::kLobbyLeft, request_id);
            } else {
                response = MakeResponse(ws::ServerAction::kLobbyEvicted);
                response["reason"] = "Kicked by host";
            }
            
            // Send notice directly down the target's pipeline
            member_it->socket->send(response.dump(), uWS::OpCode::TEXT);
        }

        // 3. Actually clear them from memory
        lobby.members.erase(member_it);
    }

    if (lobby.members.empty()) {
        code_to_id_.erase(lobby.invite_code);
        lobbies_.erase(it);
        Logger::Log("[Lobby] Destroyed lobby ", lobby_id, " (no more members)");
        return false;
    }
    return true;
}

Lobby* LobbyController::FindLobbyForUser(const string& username) {
    for (auto& [id, lobby] : lobbies_) {
        if (std::ranges::contains(lobby.members, username, &LobbyMember::username)) {
            return &lobby;
        }
    }
    return nullptr;
}
