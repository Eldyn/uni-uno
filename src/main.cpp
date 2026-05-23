#include "controllers/lobby_controller.hpp"
#include <controllers/auth_controller.hpp>
#include <common/env.hpp>
#include <logger.hpp>
#include <nlohmann/json_fwd.hpp>
#include <webserver.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    try {
        Env::Load(".env");
        WebServer server(9999);

        AuthController auth(server.GetHTTPRouter());
        LobbyController lobby(server);        

        server.GetHTTPRouter().OnAny([](AppResponse *response, AppRequest *request) {
            Logger::Log("[HTTP]: route received: ", std::string(request->getFullUrl()));
            return true;
        });

        server.GetActionRouter().OnAny([](WsContext ctx, const json& msg) -> bool {
            Logger::Log("[WS]: action received from: ", ctx.socket_data->username, " → action=", msg.value("action", "?"));
            return true;
        });
 
        server.Run();
    } catch (const std::exception& e) {
        Logger::Error(string("Fatal: ") + e.what());
        return 1;
    }
}
