#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <logger.hpp>
#include <result.hpp>
#include <websocket_context.hpp>

namespace ws {
    enum class ServerAction {
        kSuccess,           // Generic Success
                            
        kQueried,           // Send queried buttonTest information
        kSyncCount,         // Sync buttonTest information
                            
        kSyncData,          // username + room assigned after join
        kError,             // generic error with code + reason
    
        kLobbyList,         // list of open lobbies
        kLobbyUpdated,      // a lobby's state changed (player joined/left, settings changed)
        kLobbyJoined,       // you successfully joined a lobby
        kLobbyLeft,         // you or someone else left the lobby
        kInviteCreated,     // server generated an invite link token
        kLobbyStartedGame,  // server started the game
        kLobbyEvicted,      // server evicted the lobby (no players are connected/host left)
    
        kGameStateUpdate,       // full game state (your hand, top card, turn, scores)
        kGameCardRejected,      // your play_card was invalid — includes reason
        kGameCardDrawn,         // you drew a card (private — sent only to you)
        kGameTurnChanged,       // whose turn it is now
        kGameColorChanged,      // active colour changed (after wild)
        kGameUnoCallSuccess,    // a UNO call was valid
        kGameUnoCallFailed,     // a UNO call was invalid (penalised)
        kGameUnoChallenged,     // someone challenged your +4
    
        kGameStateUpdated,      // broadcast: game state changed.
        kGameCardPlayed,        // broadcast: who played what (no hand info)
        kGamePlayerDrew,        // broadcast: a player drew (count only, not what)
        kPlayerJoined,
        kPlayerLeft,
        kPlayerReady,
        kGameStarting,
        kRulesetSync,
        kGameSkipApplied,
        kGameReverseApplied,
        kGameDrawStackApplied,
        kGameOver,
        kGameRoundOver,

        kInventorySync,
        kPurchaseConfirmed,
        kPurchaseFailed,
        kChatMessage,
    };

    enum class ClientAction {
        kLobbyList,         // request current lobby list
        kLobbyCreate,       // create a new lobby with settings
        kLobbyJoin,         // join by lobby id
        kLobbyLeave,        // leave current lobby
        kLobbyInvite,       // generate an invite link for current lobby
        kLobbySetRuleset,   // host changes ruleset before game starts
        kLobbySetSkin,      // player picks a card skin for this match
        kLobbyStartGame,    // host started the game
        kReady,             // player toggles ready state
    
        kPlayCard,          // play a card from hand
        kDrawCard,          // draw from deck
        kCallUno,           // declare UNO
        kChallengeUno,      // challenge someone who forgot to call UNO
        kChallengePlusFour, // challenge a +4 (claim it was playable)
        kChooseColor,       // after playing a wild — declare colour
        kAckGameOver,       // client acknowledges the game over screen
    
        kStorePurchase,     // buy a skin or deck
        kInventoryRequest,  // fetch owned items
    
        kChatSend,          // send a chat message
    };
    
inline const std::unordered_map<ServerAction, std::string> kServerActionStr {
        { ServerAction::kSuccess,              "success"                 },
        { ServerAction::kQueried,              "queried"                 },
        { ServerAction::kSyncCount,            "sync_count"              },
        { ServerAction::kSyncData,             "sync_data"               },
        { ServerAction::kError,                "error"                   },
        { ServerAction::kLobbyList,            "lobby_list"              },
        { ServerAction::kLobbyEvicted,         "lobby_evicted"           },
        { ServerAction::kLobbyUpdated,         "lobby_updated"           },
        { ServerAction::kLobbyJoined,          "lobby_joined"            },
        { ServerAction::kLobbyLeft,            "lobby_left"              },
        { ServerAction::kInviteCreated,        "invite_created"          },
        { ServerAction::kPlayerJoined,         "player_joined"           },
        { ServerAction::kPlayerLeft,           "player_left"             },
        { ServerAction::kPlayerReady,          "player_ready"            },
        { ServerAction::kGameStarting,         "game_starting"           },
        { ServerAction::kRulesetSync,          "ruleset_sync"            },
        
        // --- The Game Prefixed String Maps ---
        { ServerAction::kGameStateUpdated,     "game_state_updated"      },
        { ServerAction::kGameCardRejected,     "game_card_rejected"      },
        { ServerAction::kGameCardDrawn,        "game_card_drawn"         },
        { ServerAction::kGameTurnChanged,      "game_turn_changed"       },
        { ServerAction::kGameColorChanged,     "game_color_changed"      },
        { ServerAction::kGameUnoCallSuccess,   "game_uno_call_success"   },
        { ServerAction::kGameUnoCallFailed,    "game_uno_call_failed"    },
        { ServerAction::kGameUnoChallenged,    "game_uno_challenged"     },
        { ServerAction::kGameCardPlayed,       "game_card_played"        },
        { ServerAction::kGamePlayerDrew,       "game_player_drew"        },
        { ServerAction::kGameSkipApplied,      "game_skip_applied"       },
        { ServerAction::kGameReverseApplied,   "game_reverse_applied"    },
        { ServerAction::kGameDrawStackApplied, "game_draw_stack_applied" },
        { ServerAction::kGameOver,             "game_over"               },
        { ServerAction::kGameRoundOver,        "game_round_over"         },
        
        { ServerAction::kInventorySync,        "inventory_sync"          },
        { ServerAction::kPurchaseConfirmed,    "purchase_confirmed"      },
        { ServerAction::kPurchaseFailed,       "purchase_failed"         },
        { ServerAction::kChatMessage,          "chat_message"            },
    };

    inline const std::unordered_map<ClientAction, std::string> kClientActionStr {
        { ClientAction::kLobbyList,         "lobby_list"          },
        { ClientAction::kLobbyCreate,       "lobby_create"        },
        { ClientAction::kLobbyJoin,         "lobby_join"          },
        { ClientAction::kLobbyLeave,        "lobby_leave"         },
        { ClientAction::kLobbyInvite,       "lobby_invite"        },
        { ClientAction::kLobbyStartGame,    "lobby_start_game"    },
        { ClientAction::kLobbySetRuleset,   "lobby_set_ruleset"   },
        { ClientAction::kLobbySetSkin,      "lobby_set_skin"      },
        { ClientAction::kReady,             "ready"               },
        { ClientAction::kPlayCard,          "play_card"           },
        { ClientAction::kDrawCard,          "draw_card"           },
        { ClientAction::kCallUno,           "call_uno"            },
        { ClientAction::kChallengeUno,      "challenge_uno"       },
        { ClientAction::kChallengePlusFour, "challenge_plus_four" },
        { ClientAction::kChooseColor,       "choose_color"        },
        { ClientAction::kAckGameOver,       "ack_game_over"       },
        { ClientAction::kStorePurchase,     "store_purchase"      },
        { ClientAction::kInventoryRequest,  "inventory_request"   },
        { ClientAction::kChatSend,          "chat_send"           },
    };
    
    
    // Reverse lookup — used in ActionRouter registration
    inline const std::unordered_map<std::string, ClientAction> kStrClientAction {
        { "lobby_list",          ClientAction::kLobbyList         },
        { "lobby_create",        ClientAction::kLobbyCreate       },
        { "lobby_join",          ClientAction::kLobbyJoin         },
        { "lobby_leave",         ClientAction::kLobbyLeave        },
        { "lobby_invite",        ClientAction::kLobbyInvite       },
        { "lobby_set_ruleset",   ClientAction::kLobbySetRuleset   },
        { "lobby_set_skin",      ClientAction::kLobbySetSkin      },
        { "game_play_card",      ClientAction::kPlayCard          },
        { "game_draw_card",      ClientAction::kDrawCard          },
        { "game_call_uno",       ClientAction::kCallUno           },
        { "game_submit_input",   ClientAction::kChooseColor       },
        { "ready",               ClientAction::kReady             },
        { "play_card",           ClientAction::kPlayCard          },
        { "draw_card",           ClientAction::kDrawCard          },
        { "call_uno",            ClientAction::kCallUno           },
        { "challenge_uno",       ClientAction::kChallengeUno      },
        { "challenge_plus_four", ClientAction::kChallengePlusFour },
        { "choose_color",        ClientAction::kChooseColor       },
        { "ack_game_over",       ClientAction::kAckGameOver       },
        { "store_purchase",      ClientAction::kStorePurchase     },
        { "inventory_request",   ClientAction::kInventoryRequest  },
        { "chat_send",           ClientAction::kChatSend          },
    };

    /**
     * @brief Extracts a mandatory field from JSON payload, returning a monadic Result.
     * 
     * Validates structural prerequisites. Maps any parsing deviations or structural 
     * missing fields straight into an Error::InvalidInput type context[cite: 2].
     * 
     * @tparam T The targeted destination data type.
     * @param json Inbound untrusted JSON payload from client channel.
     * @param key Target string data label name.
     * @return Result<T> Holding the type-verified object value or a valid error object state[cite: 1].
     */
    template <typename T>
    inline Result<T> Get(const nlohmann::json& json, std::string_view key) {
        auto it = json.find(key);
        if (it == json.end()) {
            return std::unexpected(Error::InvalidInput("Missing required field: '" + std::string(key) + "'"));
        }
        if (it->is_null()) {
            return std::unexpected(Error::InvalidInput("Required field '" + std::string(key) + "' cannot be null"));
        }

        if constexpr (std::is_same_v<T, std::string>) {
            if (it->is_string()) return it->get<std::string>();
            if (it->is_number_integer()) return std::to_string(it->get<int64_t>());
        }
        else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            if (it->is_number_integer()) return it->get<T>();
            if (it->is_string()) {
                try {
                    if constexpr (std::is_signed_v<T>) return static_cast<T>(std::stoll(it->get<std::string>()));
                    else return static_cast<T>(std::stoull(it->get<std::string>()));
                } catch (...) {}
            }
        }
        else if constexpr (std::is_same_v<T, bool>) {
            if (it->is_boolean()) return it->get<bool>();
        }
        else if constexpr (std::is_floating_point_v<T>) {
            if (it->is_number()) return it->get<T>();
        }

        return std::unexpected(Error::InvalidInput("Type mismatch for field: '" + std::string(key) + "'"));
    }

    /**
     * @brief Extracts an optional field from JSON payload with a safe fallback default.
     * 
     * Runs key discovery. If it misses or contains an incompatible data structure, 
     * execution skips error tracking paths entirely and safely steps back to default_value.
     * 
     * @tparam T Desired parameter data type (automatically inferred).
     * @param json Inbound untrusted JSON payload from client channel.
     * @param key Target string data label name.
     * @param default_value The safe fallback variable state configuration value.
     * @return T The extracted value on extraction validation success, or default_value on verification mismatch.
     */
    template <typename T>
    inline T GetOr(const nlohmann::json& json, std::string_view key, const T& default_value) {
        auto res = Get<T>(json, key);
        return res ? *res : default_value;
    }

    inline nlohmann::json MakeResponse(ws::ServerAction action, const std::string& request_id = "") {
        // Force nlohmann::json to treat this strictly as an object map
        nlohmann::json json = nlohmann::json::object({
            {"action", kServerActionStr.at(action)}
        });
    
        if (!request_id.empty()) {
            json["request_id"] = request_id;
        }
    
        Logger::Log("[WS] response sent: ", kServerActionStr.at(action), "(", request_id, ")");
        return json;
    }

    inline void SendError(AppWebSocket* ws, uWS::OpCode op, const std::string& reason, const std::string& request_id) {
        auto msg = MakeResponse(ServerAction::kError, request_id);
        msg["reason"] = reason;
        ws->send(msg.dump(), op);
    };
}
