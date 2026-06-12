#pragma once
#include <router.hpp>
#include <websocket_context.hpp>
#include <functional>
#include <string>
#include <vector>

/**
 * @file http_router.hpp
 * @brief Implementazione del router per le richieste HTTP/REST.
 * * Accumula le rotte durante la fase di avvio e le applica (Attach) in blocco 
 * all'istanza dell'applicazione uWebSockets prima dell'esecuzione.
 */

/**
 * @typedef HttpHandler
 * @brief Firma per i gestori delle richieste HTTP.
 * Riceve i puntatori agli oggetti `AppResponse` e `AppRequest` di uWS.
 * @tag HTTP-RTR-TYP-001
 */
using HttpHandler = std::function<void(AppResponse*, AppRequest*)>;

/**
 * @class HttpRouter
 * @brief Gestisce la registrazione e il routing delle chiamate HTTP (GET, POST e middleware).
 * * L'approccio differisce leggermente dall'ActionRouter perché uWebSockets richiede
 * di registrare esplicitamente le rotte HTTP direttamente sulla sua istanza nativa.
 * @tag HTTP-RTR-CLS-001
 */
class HttpRouter : public Router {
public:
    HttpRouter() = default;

    /**
     * @brief Registra un gestore per una rotta in GET.
     * Supporta i percorsi con wildcard nativi di uWS (es. "/\*").
     * @param path Il percorso URI, deve iniziare con "/".
     * @param handler La funzione da eseguire.
     * @return HttpRouter& Riferimento per il chaining.
     * @tag HTTP-RTR-MTH-001
     */
    HttpRouter& Get(const std::string& path, HttpHandler handler);

    /**
     * @brief Registra un gestore per una rotta in POST.
     * @param path Il percorso URI, deve iniziare con "/".
     * @param handler La funzione da eseguire.
     * @return HttpRouter& Riferimento per il chaining.
     * @tag HTTP-RTR-MTH-002
     */
    HttpRouter& Post(const std::string& path, HttpHandler handler);

    /**
     * @brief Registra un middleware che viene eseguito prima di qualsiasi richiesta HTTP.
     * Molto utile per configurare logging globali o Header CORS.
     * * ATTENZIONE: Se restituisce false per interrompere la catena, il middleware
     * deve occuparsi direttamente di inviare la risposta HTTP al client.
     * @param handler La funzione middleware.
     * @return HttpRouter& Riferimento per il chaining.
     * @tag HTTP-RTR-MTH-003
     */
    HttpRouter& OnAny(std::function<bool(AppResponse*, AppRequest*)> handler);

    /**
     * @brief Applica tutte le rotte accumulate (ed i relativi middleware) all'istanza nativa di uWebSockets.
     * Deve essere chiamato esattamente una volta dal `WebServer` prima di `Run()`.
     * @param app Riferimento all'istanza uWebSockets corrente.
     * @tag HTTP-RTR-MTH-004
     */
    void Attach(uWS::SSLApp& app);

private:
    /**
     * @struct Route
     * @brief Struttura interna che memorizza temporaneamente le rotte prima del caricamento.
     * @tag HTTP-RTR-STR-001
     */
    struct Route {
        std::string method;   /**< Metodo HTTP ("GET" o "POST"). */
        std::string path;     /**< Percorso URI registrato. */
        HttpHandler handler;  /**< Funzione di callback associata. */

        Route(std::string m, std::string p, HttpHandler h) : method(m), path(p), handler(h) {}
    };

    std::vector<Route>                                          routes_;
    std::vector<std::function<bool(AppResponse*, AppRequest*)>> wildcards_;

    /**
     * @brief Avvolge un handler specifico affinché esegua prima le wildcard registrate.
     * @param handler L'handler originale da decorare.
     * @return HttpHandler L'handler impacchettato con la logica dei middleware.
     * @tag HTTP-RTR-PRIV-001
     */
    HttpHandler WrapWithWildcards(HttpHandler handler);
};
