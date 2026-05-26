#include "webserver.hpp"
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
using std::string, std::vector, std::runtime_error, std::erase_if, std::memory_order_relaxed, std::transform, std::to_string, std::move;

// ---------------------------------------------------------------------------
//  Invite code alphabet — A-Z + 0-9 = 36 chars.
//  6 characters → 36^6 = ~2.18 billion combinations.
//  At one guess per second with no rate limiting that's 69 years to exhaust.
//  In practice lobbies live for ~30 minutes, making exhaustion impossible.
// ---------------------------------------------------------------------------
static constexpr char kCodeAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static constexpr int  kCodeLen        = 6;
static constexpr int  kAlphabetLen    = 36;   // strlen(kCodeAlphabet)

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

    server.OnConnectionOpen([this](AppWebSocket* ws, PerSocketData* sd) {
        this->OnOpen(ws, sd);
    });

    server.OnConnectionClose([this](AppWebSocket* ws, PerSocketData* sd) {
        this->OnClose(ws, sd);
    });
    // uWS::Loop::get() returns the event loop for the current thread.
    // createTimer() allocates a libuv timer managed by uWS.
    // The lambda fires every 1000 ms and walks all lobbies looking for
    // members whose grace period has elapsed.
    eviction_timer_ = us_create_timer((us_loop_t*)uWS::Loop::get(), 0, sizeof(LobbyController*));

    // Store `this` in the timer's extra data so the static callback can
    // reach the controller instance.
    *(LobbyController**)us_timer_ext(eviction_timer_) = this;

    us_timer_set(eviction_timer_, [](us_timer_t* t) {
        // Retrieve `this` from the extra data slot.
        auto* self = *(LobbyController**)us_timer_ext(t);
        auto  now  = steady_clock::now();

        // Collect lobby IDs where eviction happened so we can broadcast
        // after modifying the container (avoids iterator invalidation).
        vector<uint32_t> to_broadcast;
        vector<uint32_t> to_destroy;

        for (auto& [id, lobby] : self->lobbies_) {
            bool changed = false;

            // Erase members whose grace window has closed.
            erase_if(lobby.members, [&](const LobbyMember& m) {
                if (!m.is_connected) {
                    auto elapsed = duration_cast<milliseconds>(
                        now - m.disconnected_at).count();
                    if (elapsed > kReconnectGraceMs) {
                        Logger::Log("[Lobby] Evicted ", m.username, " from lobby ", id, " (grace expired)");
                        changed = true;
                        return true;
                    }
                }
                return false;
            });

            // NOTE: This does not handle the "host quit, so delete it".
            //       Which will be a good thing once we handle host passing.
            if (changed) {
                if (lobby.members.empty()) {
                    to_destroy.push_back(id);
                } else {
                    to_broadcast.push_back(id);
                }
            }
        }

        // Broadcast updates for lobbies that lost members but still exist.
        for (uint32_t id : to_broadcast) {
            self->BroadcastUpdate(self->lobbies_.at(id));
        }

        // Destroy empty lobbies and clean up the secondary index.
        for (uint32_t id : to_destroy) {
            const auto& lobby = self->lobbies_.at(id);
            self->code_to_id_.erase(lobby.invite_code);
            self->lobbies_.erase(id);
            Logger::Log("[Lobby] Destroyed empty lobby ", id);
        }

    }, 1000, 1000);   // first fire after 1000 ms, repeat every 1000 ms

    Logger::Info("[Lobby] Registered — grace window: " + to_string(kReconnectGraceMs / 1000) + "s");
}

LobbyController::~LobbyController() {
    if (eviction_timer_) {
        us_timer_close(eviction_timer_);
        eviction_timer_ = nullptr;
    }
}

//  Checks whether this username already has a slot in any lobby.  If so,
//  this is a reconnect: we swap the socket pointer, flip connected=true,
//  restore lobby_id in PerSocketData, and broadcast the update.
//
//  If not, this is a fresh connection — nothing to do here.  The client
//  will send lobby_create or lobby_join explicitly.
void LobbyController::OnOpen(AppWebSocket* ws, PerSocketData* sd) {
    Lobby* lobby = FindLobbyForUser(sd->username);
    if (!lobby) return;   // fresh connection, no existing slot

    for (auto& member : lobby->members) {
        if (member.username == sd->username) {
            Logger::Log("[Lobby] Reconnect: ", sd->username, " back in lobby ", lobby->id);

            member.socket    = ws;
            member.is_connected = true;
            // Reset the timestamp so the eviction timer ignores this member.
            member.disconnected_at = steady_clock::time_point{};

            // Restore lobby context in the socket's per-connection data
            // so OnClose can find the right lobby when this socket closes.
            sd->lobby_code = lobby->invite_code;

            BroadcastUpdate(*lobby);
            return;
        }
    }
}

//  Marks the member disconnected rather than removing them immediately.
//  The eviction timer will remove them after kReconnectGraceMs if they
//  don't reconnect in time.
void LobbyController::OnClose(AppWebSocket* ws, PerSocketData* sd) {
    if (sd->lobby_code.empty()) return;   // not in a lobby

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

    // Prevent creating a lobby while already in one.
    if (!ctx.socket_data->lobby_code.empty()) {
        ws::SendError(ctx.socket, ctx.op_code, "Already in a lobby", request_id);
        return;
    }

    // Generate a unique invite code — retry on the rare collision.
    string code;
    int attempts = 0;
    do {
        code = GenerateInviteCode();
        if (++attempts > 10)
            throw runtime_error("[Lobby] Failed to generate unique code");
    } while (code_to_id_.count(code));

    uint32_t id = next_id_.fetch_add(1, memory_order_relaxed);

    // NOTE: Constructing the lobby struct in place
    //       to avoid copies and moves :)
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
        // INFO: Who knows how we got here, but a desync happened!
        code_to_id_.erase(it); 
        ws::SendError(ctx.socket, ctx.op_code, "Lobby not found", request_id);
        return;
    }

    Lobby& lobby = lobby_it->second;
    const string& username = ctx.socket_data->username;

    // Guard against a username already in the lobby trying to join again.
    for (const auto& m : lobby.members) {
        if (m.username == username) {
            ws::SendError(ctx.socket, ctx.op_code, "Already a member", request_id);
            return;
        }
    }

    lobby.members.emplace_back(username, ctx.socket, true);
    ctx.socket_data->lobby_code = code;

    Logger::Log("[Lobby] ", username, " joined lobby ", lobby.id);

    // Send lobby_joined privately to the new member.
    auto resp = MakeResponse(ws::ServerAction::kLobbyJoined, request_id);
    resp["lobby"] = json({
        {"invite_code", code},
        {"host",        lobby.host},
        {"members",     MemberListJson(lobby)},
        {"name",        lobby.name}
    });
    ctx.socket->send(resp.dump(), ctx.op_code);

    // NOTE: Broadcast lobby_updated to everyone else so the player list refreshes.
    BroadcastUpdate(lobby);
}

//  This is the explicit reconnect action. The client sends it after the
//  WebSocket reconnects, supplying the lobby code it stored in localStorage.
//
//  Unlike HandleJoin, this succeeds only if the username is already a member
//  (possibly in disconnected state).  It does NOT add a new member slot.
//
//  OnOpen handles the socket/state restoration automatically on upgrade.
//  HandleRejoin is the explicit confirmation from the client that it wants
//  to rejoin — it receives a fresh lobby_joined payload with the current
//  member list so its UI is up to date.
void LobbyController::HandleRejoin(WsContext ctx, const json& message) {
    const std::string request_id = ws::GetOr<string>(message, "request_id", "");

    if (!message.contains("code") || !message["code"].is_string()) {
        std::cout << request_id << std::endl;
        ws::SendError(ctx.socket, ctx.op_code, "Missing lobby code", request_id);
        return;
    }

    string code = message["code"];
    transform(code.begin(), code.end(), code.begin(), ::toupper);

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) {
        // INFO: Lobby has been evicted!
        auto resp = MakeResponse(ws::ServerAction::kLobbyEvicted, request_id);
        resp["reason"] = "Lobby expired";
        ctx.socket->send(resp.dump(), ctx.op_code);
        return;
    }

    auto lobby_it = lobbies_.find(it->second);
    if (lobby_it == lobbies_.end()) {
        // INFO: Who knows how we got here, but a desync happened!
        code_to_id_.erase(it); 
        auto resp = MakeResponse(ws::ServerAction::kLobbyEvicted, request_id);
        resp["reason"] = "Lobby expired or invalid";
        ctx.socket->send(resp.dump(), ctx.op_code);
        return;
    }

    // Extract the safe reference
    Lobby& lobby = lobby_it->second;
    const string& username = ctx.socket_data->username;

    for (const auto& m : lobby.members) {
        if (m.username == username) {
            // Member found — OnOpen already restored the socket pointer.
            // Just send the current state so the client can re-render.
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
    }

    // INFO: Username not in this lobby (e.g. evicted just before rejoin arrived).
    ws::SendError(ctx.socket, ctx.op_code, "No longer a member of this lobby", request_id);
}

// Handles leave requests from the client, the leaver receives back kLobbyLeft
// with their request_id. Otherwise, for all other responses from this method,
// a on() listener is required.
void LobbyController::HandleLeave(WsContext ctx, const json& message) {
    const string& username   = ctx.socket_data->username;
    const string& code       = ctx.socket_data->lobby_code;

    const string  request_id = ws::GetOr<string>(message, "request_id", "");

    if (code.empty()) {
        ws::SendError(ctx.socket, ctx.op_code, "Not in a lobby", request_id);
        return;
    }

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) return;

    Lobby& lobby = lobbies_.at(it->second);
    uint32_t id  = lobby.id;
    bool is_host = (lobby.host == username);

    if (is_host) {
        // Host left — destroy the lobby and notify everyone.
        // Send lobby_left to all connected members before erasing.
        auto notify = MakeResponse(ws::ServerAction::kLobbyEvicted);
        notify["reason"] = "Host left";

        for (const auto& m : lobby.members) {
            if (m.is_connected && m.socket && m.username != username) {
                m.socket->send(notify.dump(), uWS::OpCode::TEXT);
            }
            // Clear everyone's lobby_id so OnClose doesn't try to operate on a
            // lobby that no longer exists.
            if (m.socket) {
                m.socket->getUserData()->lobby_code.clear();
            }
        }

        // NOTE: at this point, "code" is equal to "", so we'd end up
        //       erasing "" instead of "XXXXXX", which leaks and also
        //       crashes on rejoin (code exists lobby does not).
        code_to_id_.erase(lobby.invite_code);
        lobbies_.erase(id);

        Logger::Log("[Lobby] Destroyed lobby ", id, " (host left)");
    } else {
        if (RemoveMember(id, username)) {
            BroadcastUpdate(lobbies_.at(id));
        }
    }

    ctx.socket_data->lobby_code.clear();

    ctx.socket->send(MakeResponse(ws::ServerAction::kLobbyLeft, request_id).dump(), ctx.op_code);
}

void LobbyController::HandleList(WsContext ctx, const json& message) {
    json list = json::array();
    string request_id = ws::GetOr<string>(message, "request_id", "");

    for (const auto& [id, lobby] : lobbies_) {
        // NOTE: Only show lobbies that aren't full and have at least one connected player.
        bool any_connected = any_of(
            lobby.members.begin(), lobby.members.end(),
            [](const LobbyMember& m){ return m.is_connected; }
        );

        if (!any_connected) continue;
        if (static_cast<int>(lobby.members.size()) >= kMaxMembers) continue;
        
        // TODO: should additionally limit the list to X amount of rooms 
        //       to avoid rendering lag spikes.
       
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

//  RAND_bytes fills a buffer with cryptographically random bytes.
//  We map each byte to the alphabet using modulo.
//
//  Modulo bias: 256 % 36 = 4, so values 0-3 in the raw byte are very
//  slightly more likely than 4-35.  For a 6-char code this bias is
//  negligible (~1.5% skew on 4 of 36 chars).
//  rejection sampling for invite codes is overkill.
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

//  Sends lobby_updated only to CONNECTED members.  Disconnected members
//  will receive a fresh lobby_joined on reconnect instead.
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

// REmoves a member from the lobby.
// @return bool marking whether the lobby is still alive.
bool LobbyController::RemoveMember(uint32_t lobby_id, const string& username) {
    auto it = lobbies_.find(lobby_id);
    if (it == lobbies_.end()) return false;

    Lobby& lobby = it->second;
    erase_if(lobby.members, [&](const LobbyMember& m) {
        return m.username == username;
    });

    if (lobby.members.empty()) {
        // NOTE: no need to broadcast any data, the lobby has no users!
        code_to_id_.erase(lobby.invite_code);
        lobbies_.erase(it);

        Logger::Log("[Lobby] Destroyed lobby ", lobby_id, " (no more members)");
        return false;
    }
    return true;
}

Lobby* LobbyController::FindLobbyForUser(const string& username) {
    for (auto& [id, lobby] : lobbies_) {
        for (const auto& m : lobby.members) {
            if (m.username == username) return &lobby;
        }
    }
    return nullptr;
}
