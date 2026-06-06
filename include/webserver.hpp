#pragma once
#include <map>
#include <string>
#include <string_view>
#include <App.h>
#include <nlohmann/json.hpp>
#include <database.hpp>
#include <action_router.hpp>
#include <http_router.hpp>
#include <websocket_context.hpp>

class WebServer {
public:
    explicit WebServer(int port,
                       std::string_view keyFile  = "key.pem",
                       std::string_view certFile = "cert.pem",
                       std::string_view dbFile   = "game.db");
    ~WebServer();

    WebServer(const WebServer&)            = delete;
    WebServer& operator=(const WebServer&) = delete;

    void Run();

    ActionRouter& GetActionRouter()   { return ws_router_;   }
    HttpRouter&   GetHTTPRouter()     { return http_router_; }
    uWS::SSLApp&  GetApp()            { return app_;         }

    using ConnectionHandler = std::function<void(AppWebSocket*, PerSocketData*)>;

    void OnConnectionOpen(ConnectionHandler handler);
    void OnConnectionClose(ConnectionHandler handler);
private:
    int    port_;
    string db_file_;

    uWS::SSLApp app_;
    std::map<string, AppWebSocket*> connections_;

    ActionRouter ws_router_;
    HttpRouter   http_router_;

    bool InitDB();
    void RegisterRoutes();

    void HandlePost(AppResponse*, AppRequest*);
    void HandleGet (AppResponse*, AppRequest*);

    void OnSocketOpen   (AppWebSocket*);
    void OnSocketMessage(AppWebSocket*, std::string_view, uWS::OpCode);
    void OnSocketClosed (AppWebSocket*);

    static std::string     ReadFile    (std::string_view path);
    static std::string     GetMimeType (const string& path);

    std::vector<ConnectionHandler> on_open_hooks_;
    std::vector<ConnectionHandler> on_close_hooks_;
};
