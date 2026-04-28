#include <uWebSockets/src/App.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;

// Ogni Client avrà sta roba
struct PerSocketData {
    int user_id;
    std::string username;
};

int main() {
    const int PORT = 9999;

    uWS::App().ws<PerSocketData>("/*", {
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 16 * 1024,
        .idleTimeout = 60,

        .open = [](auto *ws) {
            std::cout << "Nuovo client connesso al server di UNO!" << std::endl;
        },

        .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
            try {
                auto data = json::parse(message);
                std::string action = data.value("action", "");

                if (action == "join") {
                    ws->getUserData()->username = data.value("username", "Anonimo");
                    std::cout << "Giocatore " << ws->getUserData()->username << " e' entrato in partita." << std::endl;
                    
                    json response = {{"status", "welcome"}, {"msg", "Benvenuto su UNO!"}};
                    ws->send(response.dump(), opCode);
                } 
                // else if (action == "play_card") {
                //     // Qui verra' inserita la verifica della correttezza delle mosse [cite: 8]
                //     std::cout << "Carta giocata: " << data["card"] << std::endl;
                //
                //     // Broadcast ai Client
                //     ws->publish("game_room", message, opCode);
                // }
            } catch (const std::exception &e) {
                std::cerr << "Errore nel parsing JSON: " << e.what() << std::endl;
            }
        },

        .close = [](auto *ws, int code, std::string_view message) {
            std::cout << "Client disconnesso." << std::endl;
        }
    }).listen(PORT, [PORT](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "✅ Server WebSocket in ascolto su ws://localhost:" << PORT << std::endl;
        }
    }).run();

    return 0;
}
