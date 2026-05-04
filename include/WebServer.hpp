#pragma once
#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <uWebSockets/src/App.h>

using json = nlohmann::json;
using std::string, std::string_view, std::ifstream, std::stringstream;

struct PerSocketData {
    string username;
    string room;
};

using AppWebSocket = uWS::WebSocket<true, true, PerSocketData>;
using AppRequest   = uWS::HttpRequest;
using AppResponse  = uWS::HttpResponse<true>;

class WebServer {
public:
    explicit WebServer(int port,
                       string_view keyFile  = "key.pem",
                       string_view certFile = "cert.pem",
                       string_view dbFile   = "game.db");
    ~WebServer();

    // Non-copyable — owns a db handle and a uWS app
    WebServer(const WebServer&)            = delete;
    WebServer& operator=(const WebServer&) = delete;

    void run();

private:
    int    port_;
    string dbFile_;

    uWS::SSLApp app_;

    sqlite3* db_ = nullptr;

    bool initDB();
    void registerRoutes();

    void handlePost(AppResponse*, AppRequest*);
    void handleGet (AppResponse*, AppRequest*);
    void handleSocketOpen   (AppWebSocket*);
    void handleSocketMessage(AppWebSocket*, string_view, uWS::OpCode);

    void ensureRoom (const string& topic);
    void incrementClicks(const string& topic);
    int  getClicks  (const string& topic);

    static string     readFile   (string_view path);
    static string     getMimeType(const string& path);
    static string     makeUsername();
};
