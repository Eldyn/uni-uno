#pragma once

/**
 * @file router.hpp
 * @brief Classe base per l'implementazione dei router (HTTP e WebSocket).
 */

/**
 * @class Router
 * @brief Interfaccia base non istanziabile per tutti i router dell'applicazione.
 * * Questa classe impone la regola di non-copiabilità. Dato che sia `ActionRouter`
 * che `HttpRouter` possiedono lo stato interno degli handler, duplicarli porterebbe
 * a comportamenti indefiniti e corruzione della memoria.
 * @tag RTR-CLS-001
 */
class Router {
public:
    virtual ~Router() = default;

    /** @brief Elimina il costruttore di copia. */
    Router(const Router&)            = delete;
    
    /** @brief Elimina l'operatore di assegnamento. */
    Router& operator=(const Router&) = delete;

protected:
    /** @brief Costruttore protetto, istanziabile solo dalle classi derivate. */
    Router() = default;
};
