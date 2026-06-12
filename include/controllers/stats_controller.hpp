#pragma once
#include <http_router.hpp>

/**
 * @file stats_controller.hpp
 * @brief Controller HTTP per la gestione delle statistiche utenti e classifiche.
 * * Gestisce richieste RESTful (GET) interagendo col Database per calcolare 
 * il ranking globale (win rate, totali partite, ecc).
 */

/**
 * @class StatsController
 * @brief Esponde gli endpoint REST necessari al frontend per il pannello Profilo e Leaderboard.
 * @tag CTRL-STATS-001
 */
class StatsController {
public:
    /**
     * @brief Costruttore del StatsController.
     * Registra le rotte HTTP (metodi GET) sull'HttpRouter fornito.
     * @param router Il router HTTP centrale dell'applicazione.
     * @tag CTRL-STATS-MTH-001
     */
    explicit StatsController(HttpRouter& router);

private:
    /**
     * @brief Handler per la rotta GET `/stats/me`.
     * * Estrae le statistiche personali e la posizione in classifica (rank)
     * del giocatore attualmente autenticato (valutando il JWT nell'header).
     * @param res Puntatore alla risposta HTTP (uWS).
     * @param req Puntatore alla richiesta HTTP (uWS).
     * @tag CTRL-STATS-ACT-001
     */
    void HandleGetMe(AppResponse* res, AppRequest* req);

    /**
     * @brief Handler per la rotta GET `/stats/leaderboard`.
     * * Esegue una query sul database per restituire la top 50 dei giocatori
     * ordinata per numero di vittorie decrescenti.
     * @param res Puntatore alla risposta HTTP.
     * @param req Puntatore alla richiesta HTTP.
     * @tag CTRL-STATS-ACT-002
     */
    void HandleGetLeaderboard(AppResponse* res, AppRequest* req);
};
