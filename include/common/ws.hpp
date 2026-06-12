#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <logger.hpp>
#include <result.hpp>
#include <websocket_context.hpp>

/**
 * @file ws.hpp
 * @brief Definizione del protocollo di messaggistica WebSocket bidirezionale.
 * * Contiene le enumerazioni delle azioni (Client -> Server e Server -> Client), 
 * costanti di mappatura stringa/enum e funzioni di utilità per l'estrazione sicura 
 * dei payload dai JSON in ingresso.
 */

namespace ws {
    /**
     * @enum ServerAction
     * @brief Azioni inviate dal Server ai Client connessi.
     * @tag WS-ENUM-001
     */
    enum class ServerAction {
        kSuccess,           /**< Successo generico con dati arbitrari allegati. */
        kError,             /**< Errore generico contenente una motivazione (reason). */
    
        kLobbyList,         /**< Invio della lista delle lobby pubbliche disponibili. */
        kLobbyUpdated,      /**< Broadcast: lo stato della lobby è cambiato. */
        kLobbyJoined,       /**< Successo: il destinatario è entrato nella lobby. */
        kLobbyLeft,         /**< Successo: il destinatario è uscito dalla lobby. */
        kLobbyEvicted,      /**< Evizione: lobby chiusa perché senza giocatori umani. */
    
        kGameStateUpdated,  /**< Broadcast: lo stato della partita in corso è cambiato. */
        kGameOver,          /**< Broadcast: la partita è terminata. */

        kChatMessage,       /**< Broadcast: un nuovo messaggio in chat. */
    };

    /**
     * @enum ClientAction
     * @brief Azioni (richieste) inviate dai Client verso il Server.
     * @tag WS-ENUM-002
     */
    enum class ClientAction {
        kLobbyStartMatch,        /**< Richiesta: Avvia la partita nella lobby corrente. */
        kLobbyList,              /**< Richiesta: Ottieni le lobby pubbliche. */
        kLobbyCreate,            /**< Richiesta: Crea una nuova lobby con impostazioni. */
        kLobbyRejoin,            /**< Richiesta: Riconnettiti a una lobby post-disconnessione. */
        kLobbyJoin,              /**< Richiesta: Unisciti a una lobby tramite codice. */
        kLobbyLeave,             /**< Richiesta: Abbandona la lobby corrente. */
        kLobbyUpdateSettings,    /**< Richiesta: Aggiorna i settings (solo Host). */
        kLobbyDeleteSavedMatch,  /**< Richiesta: Elimina partita salvata. */
        kLobbyListSavedMatches,  /**< Richiesta: Ottieni partite salvate per il gruppo attuale. */
        kLobbyResumeSavedMatch,  /**< Richiesta: Riprendi una partita salvata. */
        kLobbyPromote,           /**< Richiesta: Cedi il ruolo di Host a un altro giocatore. */
        kLobbyKick,              /**< Richiesta: Espelli un giocatore (solo Host). */
    
        kGameSubmitInput,        /**< Richiesta: Invia un input obbligatorio (es. scelta colore). */
        kGamePlayCard,           /**< Richiesta: Gioca una carta specifica. */
        kGameDrawCard,           /**< Richiesta: Pesca una carta dal mazzo. */
        kGameCallUno,            /**< Richiesta: Dichiara "UNO". */
        kGameExit,               /**< Richiesta: Esci dalla partita in corso. */
    
        kChatSend,               /**< Richiesta: Invia un messaggio nella chat della lobby. */
    };
    
    inline const std::unordered_map<ServerAction, std::string> kServerActionStr {
        { ServerAction::kSuccess,          "success"            },
        { ServerAction::kError,            "error"              },
        { ServerAction::kLobbyList,        "lobby_list"         },
        { ServerAction::kLobbyEvicted,     "lobby_evicted"      },
        { ServerAction::kLobbyUpdated,     "lobby_updated"      },
        { ServerAction::kLobbyJoined,      "lobby_joined"       },
        { ServerAction::kLobbyLeft,        "lobby_left"         },
        
        { ServerAction::kGameStateUpdated, "game_state_updated" },
        { ServerAction::kGameOver,         "game_over"          },
        
        { ServerAction::kChatMessage,      "chat_message"       },
    };

    inline const std::unordered_map<ClientAction, std::string> kClientActionStr {
        { ClientAction::kGamePlayCard,           "game_play_card"           },
        { ClientAction::kGameDrawCard,           "game_draw_card"           },
        { ClientAction::kGameCallUno,            "game_call_uno"            },

        { ClientAction::kChatSend,               "chat_send"                },

        { ClientAction::kLobbyList,              "lobby_list"               },
        { ClientAction::kLobbyStartMatch,        "lobby_start_match"        },
        { ClientAction::kLobbyCreate,            "lobby_create"             },
        { ClientAction::kLobbyRejoin,            "lobby_rejoin"             },
        { ClientAction::kLobbyJoin,              "lobby_join"               },
        { ClientAction::kLobbyLeave,             "lobby_leave"              },
        { ClientAction::kLobbyUpdateSettings,    "lobby_update_settings"    },
        { ClientAction::kLobbyDeleteSavedMatch,  "lobby_delete_saved_match" },
        { ClientAction::kLobbyListSavedMatches,  "lobby_list_saved_matches" },
        { ClientAction::kLobbyResumeSavedMatch,  "lobby_resume_saved_match" },
        { ClientAction::kLobbyPromote,           "lobby_promote"            },
        { ClientAction::kLobbyKick,              "lobby_kick"               },
    
        { ClientAction::kGameSubmitInput,        "game_submit_input"        },
        { ClientAction::kGamePlayCard,           "game_play_card"           },
        { ClientAction::kGameDrawCard,           "game_draw_card"           },
        { ClientAction::kGameCallUno,            "game_call_uno"            },
        { ClientAction::kGameExit,               "game_exit"                },
    
        { ClientAction::kChatSend,               "chat_send"                },
    };
    
    
    inline const std::unordered_map<std::string, ClientAction> kStrClientAction {
        { "lobby_list", ClientAction::kLobbyList                            },
        { "lobby_start_match", ClientAction::kLobbyStartMatch               },
        { "lobby_create", ClientAction::kLobbyCreate                        },
        { "lobby_join", ClientAction::kLobbyJoin                            },
        { "lobby_leave", ClientAction::kLobbyLeave                          },

        { "play_card", ClientAction::kGamePlayCard                          },
        { "draw_card", ClientAction::kGameDrawCard                          },
        { "call_uno", ClientAction::kGameCallUno                            },

        { "chat_send", ClientAction::kChatSend                              },

        { "lobby_list", ClientAction::kLobbyList                            },
        { "lobby_create", ClientAction::kLobbyCreate                        },
        { "lobby_rejoin", ClientAction::kLobbyRejoin                        },
        { "lobby_join", ClientAction::kLobbyJoin                            },
        { "lobby_leave", ClientAction::kLobbyLeave                          },
        { "lobby_update_settings", ClientAction::kLobbyUpdateSettings       },
        { "lobby_delete_saved_match", ClientAction::kLobbyDeleteSavedMatch  },
        { "lobby_list_saved_matches", ClientAction::kLobbyListSavedMatches  },
        { "lobby_resume_saved_match", ClientAction::kLobbyResumeSavedMatch  },
        { "lobby_promote", ClientAction::kLobbyPromote                      },
        { "lobby_kick", ClientAction::kLobbyKick                            },
    
        { "game_submit_input", ClientAction::kGameSubmitInput               },
        { "game_play_card", ClientAction::kGamePlayCard                     },
        { "game_draw_card", ClientAction::kGameDrawCard                     },
        { "game_call_uno", ClientAction::kGameCallUno                       },
        { "game_exit", ClientAction::kGameExit                              },
    
        { "chat_send", ClientAction::kChatSend                              },
    };

    /**
     * @brief Estrae un campo obbligatorio da un payload JSON, ritornando un Result monadico.
     * * Esegue validazione del tipo. Se manca il campo o il tipo è errato, restituisce 
     * un errore di tipo Error::InvalidInput.
     * * @tparam T Tipo di dato atteso (es. int, std::string).
     * @param json Payload JSON in ingresso (non fidato).
     * @param key Chiave dell'attributo da cercare.
     * @return Result<T> Il valore estratto oppure uno stato di errore incapsulato.
     * @tag WS-UTIL-001
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
     * @brief Estrae un campo opzionale da un JSON, applicando un fallback in caso di assenza/errore.
     * * @tparam T Tipo di dato atteso.
     * @param json Payload JSON.
     * @param key Chiave dell'attributo da cercare.
     * @param default_value Valore di fallback restituito se il dato è assente/non valido.
     * @return T Il valore estratto o il fallback fornito.
     * @tag WS-UTIL-002
     */
    template <typename T>
    inline T GetOr(const nlohmann::json& json, std::string_view key, const T& default_value) {
        auto res = Get<T>(json, key);
        return res ? *res : default_value;
    }

    /**
     * @brief Costruisce l'intelaiatura di base per un messaggio JSON di risposta del server.
     * @param action Il tipo di azione (ServerAction) da inserire nel payload.
     * @param request_id (Opzionale) ID della richiesta originale per il tracciamento frontend.
     * @return nlohmann::json Struttura JSON pronta per essere popolata con altri campi.
     * @tag WS-UTIL-003
     */
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

    /**
     * @brief Invia un payload JSON formattato come errore sul socket specifo.
     * @param ws Puntatore al socket destinatario.
     * @param op L'OpCode di uWebSockets (es. testo).
     * @param reason La stringa descrittiva dell'errore (motivo).
     * @param request_id (Opzionale) L'ID del pacchetto che ha scatenato l'errore.
     * @tag WS-UTIL-004
     */
    inline void SendError(AppWebSocket* ws, uWS::OpCode op, const std::string& reason, const std::string& request_id) {
        auto msg = MakeResponse(ServerAction::kError, request_id);
        msg["reason"] = reason;
        ws->send(msg.dump(), op);
    };
}
