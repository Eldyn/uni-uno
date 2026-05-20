#pragma once
#include <string>
#include <App.h>

// Per-socket state — stored inside each uWS connection.
// uWS owns this memory for the lifetime of the socket.
struct PerSocketData {
    std::string username;   // set on upgrade after JWT verification
    std::string room;       // set on "join" action
    std::string lobby_id;
};

using AppWebSocket = uWS::WebSocket<true, true, PerSocketData>;
using AppRequest   = uWS::HttpRequest;
using AppResponse  = uWS::HttpResponse<true>;

// Passed to every ActionRouter handler.
// Bundles everything a handler needs — no global state required.
struct WsContext {
    AppWebSocket*  socket;
    PerSocketData* socket_data;
    uWS::OpCode    op_code;
};
