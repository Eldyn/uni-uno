#include <transport/uws_broadcaster.hpp>

void UwsBroadcaster::Send(AppWebSocket* socket, const std::string& msg,
                           uWS::OpCode op, bool compress) {
    socket->send(msg, op, compress);
}

void UwsBroadcaster::Publish(const std::string& topic, const std::string& msg,
                              uWS::OpCode op, bool compress) {
    app_.publish(topic, msg, op, compress);
}

void UwsBroadcaster::Subscribe(AppWebSocket* socket, const std::string& topic) {
    socket->subscribe(topic);
}

void UwsBroadcaster::Unsubscribe(AppWebSocket* socket, const std::string& topic) {
    socket->unsubscribe(topic);
}
