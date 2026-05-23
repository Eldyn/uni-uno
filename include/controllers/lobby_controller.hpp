#pragma once
#include <common/ws.hpp>
#include <atomic>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <App.h>
#include <action_router.hpp>
#include <websocket_context.hpp>
#include <webserver.hpp>

//  Owns all in-memory lobby state. No database involved — lobbies are
//  ephemeral session state that dies with the server process.
//
//  Lobby identity:
//    Each lobby has two identifiers:
//      id           — uint32_t, atomically incremented, used internally and
//                     in WebSocket messages to already-authenticated clients.
//                     Never appears in invite links or URLs.
//      invite_code  — 6-character uppercase alphanumeric string generated
//                     from RAND_bytes.  This is the only thing shared with
//                     players.  128-bit-equivalent guessing resistance at
//                     game scale; immune to +1 enumeration attacks.
//
//  Reconnection:
//    When a socket closes, the member is marked disconnected and a timestamp
//    is recorded.  A repeating timer (uWS Loop timer, 1s interval) evicts
//    members whose grace period (kReconnectGraceMs) has elapsed.
//
//    When a new socket opens for a username that is already present in a
//    lobby, the socket pointer is swapped and the member is marked connected
//    again.  The client must send lobby_rejoin with the stored lobby code
//    (kept in localStorage) to restore their lobby_id in PerSocketData.
struct LobbyMember {
    std::string     username;
    AppWebSocket*   socket;        // nullptr when disconnected
    bool            is_connected;
    
    std::chrono::steady_clock::time_point disconnected_at{};

    LobbyMember(std::string u, AppWebSocket* s, bool c) 
        : username(std::move(u)), socket(s), is_connected(c)  {}
};

struct Lobby {
    uint32_t                 id;
    bool                     is_public;
    std::string              invite_code;   // 6-char A-Z0-9, the join token
    std::string              host;          // username of creator
    std::vector<LobbyMember> members;
    std::string              name;
    // TODO: Additionally, we will hold some other useful data, for example, creation_time.
};

class LobbyController {
public:
    // Registers WebSocket action handlers on `router`.
    // Holds a reference to `app` for pub/sub broadcasts and the eviction timer.
    // Both must outlive this controller.
    LobbyController(WebServer& webserver);
    ~LobbyController();

    // Called by WebServer::OnSocketOpen after JWT verification.
    // If `username` already has a slot in a lobby (reconnect scenario),
    // the socket pointer is restored and lobby_id is written into `sd`.
    void OnOpen(AppWebSocket* ws, PerSocketData* sd);

    // Called by WebServer::OnSocketClosed.
    // Marks the member disconnected and records the timestamp.
    // Does NOT immediately remove — the grace timer handles eviction.
    void OnClose(AppWebSocket* ws, PerSocketData* sd);

    // Grace period before a disconnected member is evicted 3 minutes in milliseconds.
    // static constexpr int kReconnectGraceMs = 1'000 * 60 * 3;
    // INFO: for debugging, so disgraceful!
    static constexpr int kReconnectGraceMs = 10'000;

    // Maximum players per lobby.
    static constexpr int kMaxMembers = 4;

private:
    // lobby_create  { }
    //   Creates a new lobby, makes the sender the host, responds with
    //   lobby_joined carrying the invite_code and member list.
    void HandleCreate(WsContext ctx, const json& msg);

    // lobby_join  { "code": "XK4F9Z" }
    //   Joins an existing lobby by invite code.
    void HandleJoin(WsContext ctx, const json& msg);

    // lobby_rejoin  { "code": "XK4F9Z" }
    //   Reconnect path: only succeeds if the username already has a slot in
    //   that lobby (possibly disconnected).  Restores presence without adding
    //   a new member.
    void HandleRejoin(WsContext ctx, const json& msg);

    // lobby_leave  { }
    //   Voluntarily leaves.  If the host leaves, the lobby is destroyed and
    //   all remaining members receive lobby_left.
    void HandleLeave(WsContext ctx, const json& msg);

    // lobby_list  { }
    //   Returns all lobbies that are not full and have at least one connected
    //   member.  Useful for a "browse" screen.
    void HandleList(WsContext ctx, const json& msg);

    // Generate a 6-character uppercase alphanumeric code using RAND_bytes.
    static std::string GenerateInviteCode();

    // Serialise the member list for a lobby_updated / lobby_joined payload.
    // Each entry: { "username": "...", "connected": true/false, "host": bool }
    static json MemberListJson(const Lobby& lobby);

    // Broadcast lobby_updated to every CONNECTED member of a lobby.
    void BroadcastUpdate(const Lobby& lobby) const;

    // Remove a member from a lobby by username.  Destroys the lobby if empty.
    // Returns true if the lobby still exists after removal.
    bool RemoveMember(uint32_t lobby_id, const std::string& username);

    // Find the lobby a given username currently belongs to.
    // Returns nullptr if not found.
    Lobby* FindLobbyForUser(const std::string& username);

    // ── State ────────────────────────────────────────────────────────────
    ActionRouter& action_router_;

    // Primary store: lobby_id → Lobby
    std::unordered_map<uint32_t, Lobby> lobbies_;

    // Secondary index: invite_code → lobby_id  (for O(1) join-by-code)
    std::unordered_map<std::string, uint32_t> code_to_id_;

    // Atomically-incrementing lobby ID counter.
    // fetch_add with memory_order_relaxed is correct here: we only need
    // uniqueness, not ordering relative to other memory operations.
    std::atomic<uint32_t> next_id_{1};

    // uWS repeating timer handle — used for grace-period eviction.
    // Stored so we can cancel it in the destructor.
    us_timer_t* eviction_timer_ = nullptr;
};
