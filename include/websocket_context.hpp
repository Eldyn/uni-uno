#pragma once
#include <string>
#include <App.h>

/**
 * @file websocket_context.hpp
 * @brief Definisce i tipi e le strutture di contesto associati alle connessioni WebSocket.
 * * Questo file fa da ponte tra la libreria uWebSockets e la logica applicativa,
 * definendo i dati che vengono mantenuti in memoria per ogni singolo socket connesso.
 */

/**
 * @struct PerSocketData
 * @brief Dati di stato mantenuti all'interno di ciascuna connessione uWS.
 * * La libreria uWebSockets detiene la proprietà della memoria di questa struttura
 * per l'intera durata della vita del socket. Viene usata per tracciare l'identità
 * dell'utente connesso senza dover usare mappe di lookup esterne.
 * @tag WS-CTX-STR-001
 */
struct PerSocketData {
    std::string username;   /**< Username impostato post-upgrade dopo la verifica del JWT. */
    std::string room;       /**< Stanza di broadcast uWS corrente (es. ID della lobby). */
    std::string lobby_code; /**< Il codice invito della lobby in cui si trova l'utente. */
};

/**
 * @typedef AppWebSocket
 * @brief Alias per il tipo complesso di socket uWebSockets configurato per SSL e routing.
 * @tag WS-CTX-TYP-001
 */
using AppWebSocket = uWS::WebSocket<true, true, PerSocketData>;

/**
 * @typedef AppRequest
 * @brief Alias per la richiesta HTTP in ingresso di uWebSockets.
 * @tag WS-CTX-TYP-002
 */
using AppRequest   = uWS::HttpRequest;

/**
 * @typedef AppResponse
 * @brief Alias per la risposta HTTP in uscita di uWebSockets (con supporto SSL).
 * @tag WS-CTX-TYP-003
 */
using AppResponse  = uWS::HttpResponse<true>;

/**
 * @struct WsContext
 * @brief Contesto di esecuzione passato ad ogni gestore del `ActionRouter`.
 * * Incapsula tutto ciò di cui un handler ha bisogno per processare un messaggio e
 * rispondere, eliminando la necessità di accedere a stati globali.
 * @tag WS-CTX-STR-002
 */
struct WsContext {
    AppWebSocket* socket;       /**< Puntatore al socket che ha inviato il messaggio. */
    PerSocketData* socket_data;  /**< Dati di sessione associati al socket chiamante. */
    uWS::OpCode    op_code;      /**< Codice operativo del frame WebSocket (Es. Testo o Binario). */
};
