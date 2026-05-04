#include "../include/WebServer.hpp"
#include "../include/Logger.hpp"

int main() {
    try {
        WebServer server(9999);
        server.run();
    } catch (const std::exception& e) {
        Logger::error(string("Fatal: ") + e.what());
        return 1;
    }
    return 0;
}
