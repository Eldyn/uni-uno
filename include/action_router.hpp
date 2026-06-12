#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
#include <router.hpp>
#include <websocket_context.hpp>

/**
 * @file action_router.hpp
 * @brief Implementazione del router per lo smistamento dei messaggi WebSocket.
 * * Indirizza i messaggi JSON in arrivo ai gestori appropriati ispezionando
 * il campo "action" all'interno del payload.
 */

using json = nlohmann::json;

/**
 * @typedef ActionHandler
 * @brief Firma per i gestori delle azioni WebSocket.
 * * Restituisce un booleano:
 * - `true`: Continua il dispatch (passa al gestore specifico successivo).
 * - `false`: Interrompe il dispatch (usato dai middleware/wildcard per bloccare l'azione, es. guardia di autenticazione).
 * @tag ACT-RTR-TYP-001
 */
using ActionHandler = std::function<bool(WsContext, const json&)>;

/**
 * @class ActionRouter
 * @brief Smista i messaggi WebSocket ai vari controller in base al tipo di azione.
 * * Supporta l'uso di "wildcard" (middleware) che vengono eseguiti prima di ogni azione
 * specifica. Se una wildcard restituisce `false`, il gestore specifico non viene mai chiamato.
 * Eredita da `Router` per disabilitare la copia.
 * @tag ACT-RTR-CLS-001
 */
class ActionRouter : public Router {
public:
    ActionRouter() = default;

    /**
     * @brief Registra un gestore per una specifica azione.
     * @param action Il nome dell'azione (es. "play_card").
     * @param handler La funzione da eseguire quando si riceve l'azione.
     * @return ActionRouter& Riferimento a se stesso per il concatenamento (chaining).
     * @tag ACT-RTR-MTH-001
     */
    ActionRouter& On(const std::string& action, ActionHandler handler);

    /**
     * @brief Registra un gestore "wildcard" (middleware).
     * I middleware vengono eseguiti nell'ordine di registrazione prima di ogni handler specifico.
     * @param handler La funzione da eseguire. Se restituisce false, blocca la catena.
     * @return ActionRouter& Riferimento a se stesso.
     * @tag ACT-RTR-MTH-002
     */
    ActionRouter& OnAny(ActionHandler handler);

    /**
     * @brief Avvia lo smistamento di un messaggio in arrivo.
     * * Viene chiamato internamente dal `WebServer` su ogni frame WebSocket ricevuto.
     * Esegue prima le wildcard; se tutte restituiscono true, cerca ed esegue il gestore specifico.
     * @param ctx Il contesto della connessione WebSocket.
     * @param msg Il payload JSON ricevuto dal client.
     * @return true se un gestore specifico è stato trovato ed eseguito, false se non esiste alcun gestore (dopo aver eseguito le wildcard).
     * @tag ACT-RTR-MTH-003
     */
    bool Dispatch(WsContext ctx, const json& msg) const;

private:
    std::unordered_map<std::string, ActionHandler> handlers_; /**< Mappa delle azioni ai relativi handler specifici. */

    std::vector<ActionHandler> wildcards_; /**< Lista dei middleware eseguiti prima di ogni azione specifica. */
};
