#include <controllers/lobby_controller.hpp>
#include <common/ws.hpp>
#include <logger.hpp>
#include <openssl/rand.h>
#include <algorithm>
#include <chrono>
#include <stdexcept>

using namespace std::chrono;

// ---------------------------------------------------------------------------
//  Invite code alphabet — A-Z + 0-9 = 36 chars.
//  6 characters → 36^6 = ~2.18 billion combinations.
//  At one guess per second with no rate limiting that's 69 years to exhaust.
//  In practice lobbies live for ~30 minutes, making exhaustion impossible.
// ---------------------------------------------------------------------------
static constexpr char kCodeAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static constexpr int  kCodeLen        = 6;
static constexpr int  kAlphabetLen    = 36;   // strlen(kCodeAlphabet)

// ---------------------------------------------------------------------------
//  Constructor
// ---------------------------------------------------------------------------

LobbyController::LobbyController(ActionRouter& router, uWS::SSLApp& app) : action_router_(router), app_(app) {

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
        std::vector<uint32_t> to_broadcast;
        std::vector<uint32_t> to_destroy;

        for (auto& [id, lobby] : self->lobbies_) {
            bool changed = false;

            // Erase members whose grace window has closed.
            std::erase_if(lobby.members, [&](const LobbyMember& m) {
                if (!m.connected) {
                    auto elapsed = duration_cast<milliseconds>(
                        now - m.disconnected_at).count();
                    if (elapsed > kReconnectGraceMs) {
                        Logger::Log("[Lobby] Evicted ", m.username,
                                    " from lobby ", id, " (grace expired)");
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

    Logger::Info("[LobbyController] Registered — grace window: " +
                 std::to_string(kReconnectGraceMs / 1000) + "s");
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
            Logger::Log("[Lobby] Reconnect: ", sd->username,
                        " back in lobby ", lobby->id);

            member.socket    = ws;
            member.connected = true;
            // Reset the timestamp so the eviction timer ignores this member.
            member.disconnected_at = steady_clock::time_point{};

            // Restore lobby context in the socket's per-connection data
            // so OnClose can find the right lobby when this socket closes.
            sd->lobby_id = lobby->invite_code;

            BroadcastUpdate(*lobby);
            return;
        }
    }
}

//  Marks the member disconnected rather than removing them immediately.
//  The eviction timer will remove them after kReconnectGraceMs if they
//  don't reconnect in time.
void LobbyController::OnClose(AppWebSocket* ws, PerSocketData* sd) {
    if (sd->lobby_id.empty()) return;   // not in a lobby

    auto it = code_to_id_.find(sd->lobby_id);
    if (it == code_to_id_.end()) return;

    auto lobby_it = lobbies_.find(it->second);
    if (lobby_it == lobbies_.end()) return;

    Lobby& lobby = lobby_it->second;

    for (auto& member : lobby.members) {
        if (member.username == sd->username) {
            Logger::Log("[Lobby] Disconnect: ", sd->username,
                        " in lobby ", lobby.id, " — grace window open");
            member.connected       = false;
            member.socket          = nullptr;
            member.disconnected_at = steady_clock::now();
            BroadcastUpdate(lobby);
            return;
        }
    }
}

void LobbyController::HandleCreate(WsContext ctx, const json& message) {
    const std::string& username = ctx.socket_data->username;

    // Prevent creating a lobby while already in one.
    if (!ctx.socket_data->lobby_id.empty()) {
        ws::SendError(ctx.socket, ctx.op_code, "Already in a lobby");
        return;
    }

    // Generate a unique invite code — retry on the rare collision.
    std::string code;
    int attempts = 0;
    do {
        code = GenerateInviteCode();
        if (++attempts > 10)
            throw std::runtime_error("[Lobby] Failed to generate unique code");
    } while (code_to_id_.count(code));

    uint32_t id = next_id_.fetch_add(1, std::memory_order_relaxed);

    Lobby lobby;
    lobby.id          = id;
    lobby.invite_code = code;
    lobby.host        = username;
    lobby.members.push_back({username, ctx.socket, true, {}});

    code_to_id_[code] = id;
    lobbies_[id]      = std::move(lobby);

    // Write the lobby code into the socket's per-connection state so
    // OnClose can find the right lobby when this socket eventually closes.
    ctx.socket_data->lobby_id = code;

    Logger::Log("[Lobby] Created lobby ", id, " code=", code,
                " host=", username);

    // Respond with lobby_joined — carries enough info to render the lobby UI.
    auto resp = MakeResponse(ws::ServerAction::kLobbyJoined, message["request_id"]);
    resp["lobby_id"]     = id;
    resp["invite_code"]  = code;
    resp["members"]      = MemberListJson(lobbies_.at(id));
    ctx.socket->send(resp.dump(), ctx.op_code);
}

void LobbyController::HandleJoin(WsContext ctx, const json& message) {
    if (!message.contains("code") || !message["code"].is_string()) {
        ws::SendError(ctx.socket, ctx.op_code, "Missing invite code");
        return;
    }

    if (!ctx.socket_data->lobby_id.empty()) {
        ws::SendError(ctx.socket, ctx.op_code, "Already in a lobby");
        return;
    }

    std::string code = message["code"];
    // Normalise to uppercase so "xk4f9z" and "XK4F9Z" both work.
    std::transform(code.begin(), code.end(), code.begin(), ::toupper);

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) {
        ws::SendError(ctx.socket, ctx.op_code, "Lobby not found");
        return;
    }

    Lobby& lobby = lobbies_.at(it->second);

    if (static_cast<int>(lobby.members.size()) >= kMaxMembers) {
        ws::SendError(ctx.socket, ctx.op_code, "Lobby is full");
        return;
    }

    const std::string& username = ctx.socket_data->username;

    // Guard against a username already in the lobby trying to join again.
    for (const auto& m : lobby.members) {
        if (m.username == username) {
            ws::SendError(ctx.socket, ctx.op_code, "Already a member");
            return;
        }
    }

    lobby.members.push_back({username, ctx.socket, true, {}});
    ctx.socket_data->lobby_id = code;

    Logger::Log("[Lobby] ", username, " joined lobby ", lobby.id);

    // Send lobby_joined privately to the new member.
    auto resp = MakeResponse(ws::ServerAction::kLobbyJoined, message["request_id"]);
    resp["lobby_id"]    = lobby.id;
    resp["invite_code"] = code;
    resp["members"]     = MemberListJson(lobby);
    ctx.socket->send(resp.dump(), ctx.op_code);

    // Broadcast lobby_updated to everyone else so the player list refreshes.
    BroadcastUpdate(lobby);
}

//  This is the explicit reconnect action.  The client sends it after the
//  WebSocket reconnects, supplying the lobby code it stored in sessionStorage.
//
//  Unlike HandleJoin, this succeeds only if the username is already a member
//  (possibly in disconnected state).  It does NOT add a new member slot.
//
//  OnOpen handles the socket/state restoration automatically on upgrade.
//  HandleRejoin is the explicit confirmation from the client that it wants
//  to rejoin — it receives a fresh lobby_joined payload with the current
//  member list so its UI is up to date.
void LobbyController::HandleRejoin(WsContext ctx, const json& message) {
    if (!message.contains("code") || !message["code"].is_string()) {
        ws::SendError(ctx.socket, ctx.op_code, "Missing lobby code");
        return;
    }

    std::string code = message["code"];
    std::transform(code.begin(), code.end(), code.begin(), ::toupper);

    auto it = code_to_id_.find(code);
    if (it == code_to_id_.end()) {
        // Lobby expired during the grace window — tell the client to go to lobby list.
        auto resp = MakeResponse(ws::ServerAction::kLobbyLeft, message["request_id"]);
        resp["reason"] = "Lobby expired";
        ctx.socket->send(resp.dump(), ctx.op_code);
        return;
    }

    Lobby& lobby = lobbies_.at(it->second);
    const std::string& username = ctx.socket_data->username;

    for (const auto& m : lobby.members) {
        if (m.username == username) {
            // Member found — OnOpen already restored the socket pointer.
            // Just send the current state so the client can re-render.
            auto resp = MakeResponse(ws::ServerAction::kLobbyJoined, message["request_id"]);
            resp["lobby_id"]    = lobby.id;
            resp["invite_code"] = code;
            resp["members"]     = MemberListJson(lobby);
            ctx.socket->send(resp.dump(), ctx.op_code);
            return;
        }
    }

    // Username not in this lobby (e.g. evicted just before rejoin arrived).
    ws::SendError(ctx.socket, ctx.op_code, "No longer a member of this lobby");
}

void LobbyController::HandleLeave(WsContext ctx, const json& message) {
    const std::string& username = ctx.socket_data->username;
    const std::string& code     = ctx.socket_data->lobby_id;

    if (code.empty()) {
        ws::SendError(ctx.socket, ctx.op_code, "Not in a lobby");
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
        auto notif = MakeResponse(ws::ServerAction::kLobbyLeft, message["request_id"]);
        notif["reason"] = "Host left";

        for (const auto& m : lobby.members) {
            if (m.connected && m.socket && m.username != username) {
                m.socket->send(notif.dump(), uWS::OpCode::TEXT);
            }
            // Clear their lobby_id so OnClose doesn't try to operate on a
            // lobby that no longer exists.
            if (m.socket) {
                m.socket->getUserData()->lobby_id.clear();
            }
        }

        code_to_id_.erase(code);
        lobbies_.erase(id);
        Logger::Log("[Lobby] Destroyed lobby ", id, " (host left)");
    } else {
        RemoveMember(id, username);
        BroadcastUpdate(lobbies_.at(id));
    }

    ctx.socket_data->lobby_id.clear();

    ctx.socket->send(
        MakeResponse(ws::ServerAction::kLobbyLeft).dump(), ctx.op_code);
}

void LobbyController::HandleList(WsContext ctx, const json& message) {
    json list = json::array();

    for (const auto& [id, lobby] : lobbies_) {
        // Only show lobbies that aren't full and have at least one connected player.
        bool any_connected = std::any_of(
            lobby.members.begin(), lobby.members.end(),
            [](const LobbyMember& m){ return m.connected; });

        if (!any_connected) continue;
        if (static_cast<int>(lobby.members.size()) >= kMaxMembers) continue;

        // NOTE: invite_code is intentionally NOT included in the list.
        //       Players must receive a code directly to join — no lobby browser
        //       should expose codes to unauthenticated enumerators.
        
        // TODO: public lobbies will later also expose the code as a join button
        
        // TODO: should additionally limit the list to X amount of rooms
        //       to avoid rendering lag spikes.
        
        list.push_back({
            {"lobby_id",     id},
            {"host",         lobby.host},
            {"player_count", lobby.members.size()},
            {"max_players",  kMaxMembers},
        });
    }

    auto resp = MakeResponse(ws::ServerAction::kLobbyList, message["request_id"]);
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
std::string LobbyController::GenerateInviteCode() {
    uint8_t raw[kCodeLen];
    if (RAND_bytes(raw, kCodeLen) != 1)
        throw std::runtime_error("[Lobby] RAND_bytes failed generating invite code");

    std::string code(kCodeLen, ' ');
    for (int i = 0; i < kCodeLen; ++i)
        code[i] = kCodeAlphabet[raw[i] % kAlphabetLen];
    return code;
}

json LobbyController::MemberListJson(const Lobby& lobby) {
    json arr = json::array();
    for (const auto& m : lobby.members) {
        arr.push_back({
            {"username",  m.username},
            {"connected", m.connected},
            {"is_host",   m.username == lobby.host},
        });
    }
    return arr;
}

//  Sends lobby_updated only to CONNECTED members.  Disconnected members
//  will receive a fresh lobby_joined on reconnect instead.
void LobbyController::BroadcastUpdate(const Lobby& lobby) const {
    auto msg = MakeResponse(ws::ServerAction::kLobbyUpdated);
    msg["lobby_id"] = lobby.id;
    msg["members"]  = MemberListJson(lobby);
    std::string payload = msg.dump();

    for (const auto& m : lobby.members) {
        if (m.connected && m.socket) {
            m.socket->send(payload, uWS::OpCode::TEXT);
        }
    }
}

bool LobbyController::RemoveMember(uint32_t lobby_id, const std::string& username) {
    auto it = lobbies_.find(lobby_id);
    if (it == lobbies_.end()) return false;

    Lobby& lobby = it->second;
    std::erase_if(lobby.members, [&](const LobbyMember& m) {
        return m.username == username;
    });

    if (lobby.members.empty()) {
        code_to_id_.erase(lobby.invite_code);
        lobbies_.erase(it);
        return false;
    }
    return true;
}

Lobby* LobbyController::FindLobbyForUser(const std::string& username) {
    for (auto& [id, lobby] : lobbies_) {
        for (const auto& m : lobby.members) {
            if (m.username == username) return &lobby;
        }
    }
    return nullptr;
}
