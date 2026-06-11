#pragma once
#include <http_router.hpp>

class StatsController {
public:
    // Registers HTTP routes on the supplied router.
    explicit StatsController(HttpRouter& router);

private:
    // GET /stats/me
    // Returns the current player's statistics and their global rank.
    void HandleGetMe(AppResponse* res, AppRequest* req);

    // GET /stats/leaderboard
    // Returns the top 50 players by wins.
    void HandleGetLeaderboard(AppResponse* res, AppRequest* req);
};
