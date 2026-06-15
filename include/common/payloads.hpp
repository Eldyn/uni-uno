#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <common/lobby.hpp>
#include <result.hpp>

namespace ws {

// Payload structs for incoming ClientAction messages.
// Note: 'request_id' and 'action' are extracted separately in the router/controller,
// so these payloads only map the action-specific data fields.
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT is used so optional fields don't throw,
// but for payloads where fields are strictly required, we could use NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE.

template <typename T>
inline Result<T> ParsePayload(const nlohmann::json& json) {
    try {
        return json.get<T>();
    } catch (const nlohmann::json::exception& e) {
        return std::unexpected(Error::InvalidInput(e.what()));
    }
}

struct LobbyJoinPayload {
    std::string code;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LobbyJoinPayload, code)

struct LobbyRejoinPayload {
    std::string code;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LobbyRejoinPayload, code)

struct LobbyCreatePayload {
    bool is_public = false;
    std::string name = "";
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LobbyCreatePayload, is_public, name)

struct LobbyPromotePayload {
    std::string username;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LobbyPromotePayload, username)

struct LobbyKickPayload {
    std::string username;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LobbyKickPayload, username)

struct LobbyUpdateSettingsPayload {
    std::optional<bool> is_public;
    std::optional<std::string> name;
    std::optional<int> bot_count;
    std::optional<int> max_score;
    std::optional<int> turn_time_limit;
    std::optional<bool> allow_bot_takeover;
    std::optional<bool> save_state;
};
// We need to define a custom from_json for optionals or use merge_patch as done currently.
// Wait, the requirement says "map json objects with NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE".
// But `std::optional` is not directly supported by `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE` without a custom serializer in older versions.
// However, LobbySettings could just be merged using json.merge_patch(message), so maybe we don't strictly need a struct for it. 
// Let's check other payloads.

struct LobbyDeleteSavedMatchPayload {
    std::string match_id;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LobbyDeleteSavedMatchPayload, match_id)

struct LobbyResumeSavedMatchPayload {
    std::string match_id;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LobbyResumeSavedMatchPayload, match_id)

struct GamePlayCardPayload {
    uint16_t card_id;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GamePlayCardPayload, card_id)

struct GameSubmitInputPayload {
    std::string value;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameSubmitInputPayload, value)

} // namespace ws
