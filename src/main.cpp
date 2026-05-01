#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <uWebSockets/src/App.h>
#include "WebSocketData.h"


namespace fs = std::filesystem;
using json = nlohmann::json;
using std::to_string, std::move, std::ios, std::ifstream, std::stringstream, std::string_view, std::exception, std::cout, std::endl, std::string;

// Ogni Client avrà sta roba
struct PerSocketData {
  string username;
  string room;
};

using AppWebSocket = uWS::WebSocket<true, true, PerSocketData>;
using AppRequest = uWS::HttpRequest;
using AppResponse = uWS::HttpResponse<true>;

string readFile(string_view path) {
    ifstream is(path.data(), ios::binary);
    if (!is) return "";
    stringstream buffer;
    buffer << is.rdbuf();
    return buffer.str();
}

string getMimeType(string path) {
    if (path.ends_with(".html")) return "text/html";
    if (path.ends_with(".js"))   return "text/javascript";
    if (path.ends_with(".css"))  return "text/css";
    if (path.ends_with(".svg"))  return "image/svg+xml";
    if (path.ends_with(".png"))  return "image/png";
    return "application/octet-stream";
};


int main() {
  srand(time(NULL));

  uWS::SocketContextOptions ssl_options;
  ssl_options.key_file_name = "key.pem";
  ssl_options.cert_file_name = "cert.pem";

  const int PORT = 9999;

  // Room to Click count
  std::map<string, int> clickCounter;

  uWS::SSLApp app = uWS::SSLApp(ssl_options);
  app.post("/room", [](AppResponse *response, AppRequest *request) {
      response->onAborted([]() {});
      string buffer;

      cout << "[POST]: /create-topic RECEIVED" << endl;
      response->onData([response, buffer = move(buffer)](string_view chunk, bool isLast) mutable {
        buffer.append(chunk.data(), chunk.length());
  
        if (isLast) {
          try {
            auto data = nlohmann::json::parse(buffer);
          
            string topic = data.value("topic", "default");
            cout << "[POST]: /create-topic DATA:" << topic << endl;
    
            response->writeHeader("Content-Type", "application/json")
                    ->end("{\"status\": \"OK\", \"topic\": \"" + topic + "\"}");
    
          } catch (const exception &e) {
            response->writeStatus("400 Bad Request")->end("Invalid JSON");
          }
        }
      });
    });
  app.get("/*", [](AppResponse *response, AppRequest *request) {
      response->onAborted([]() { /* Gestione cleanup */ });

      string url = string(request->getUrl());
      string relativePath = (url == "/") ? "index.html" : url.substr(1);

      fs::path baseDir = fs::current_path() / "public";
      fs::path filePath = baseDir / relativePath;


      if (fs::exists(filePath) && !fs::is_directory(filePath)) {
        string pathStr = filePath.string();
        string_view mime = getMimeType(pathStr);

        response->writeHeader("Content-Type", string(mime))
           ->writeHeader("X-Content-Type-Options", "nosniff")
           ->end(readFile(pathStr));
      } else {
        cout << "Tried finding: " << filePath << " but 404!" << endl;
        // Fallback su index.html per gestire il routing lato client (Svelte)
        response->writeStatus("404 Not Found")->end("File non trovato");
      }
    });
  app.ws<PerSocketData>("/*", {
      .open = [](AppWebSocket *ws) {
        PerSocketData* socketData = ws->getUserData();
  
        socketData->username = "giovannino_" + to_string(rand() % 100);
        cout << "[SOCKET]: " << socketData->username << " 101 Connection Upgraded."<< endl;
      },
      .message = [&clickCounter](AppWebSocket *ws, string_view message, uWS::OpCode opCode) {
        try {
          PerSocketData* socketData = ws->getUserData();
          auto message_json = json::parse(message);
          string action = message_json["action"];
    
          if (action == "query") {
            string topic = message_json["topic"];
            ws->send(to_string(clickCounter.at(topic)));
          }
          if (action == "join") {
            string topic = message_json["topic"];
            
            if (!socketData->room.empty()) {
              ws->send("Sei già in una room");
              return;
            }
            
            socketData->room = topic;
            cout << socketData->username << " " << socketData->room << endl;

            ws->subscribe(topic);
            ws->send("Sei iscritto alla room bro.", opCode);

            clickCounter.insert({topic, 0});

            ws->send("La room ha ben " + to_string(clickCounter.at(topic)) + " click.", opCode);

            cout << "[SOCKET]: " << socketData->username << " iscritto alla topic: " << topic << endl;
          } 
          else if (action == "click") {
            cout << socketData->room << " pre: " << clickCounter.at(socketData->room) << endl;
            clickCounter.at(socketData->room) = clickCounter.at(socketData->room) + 1;
            cout << socketData->room << " post: " << clickCounter.at(socketData->room) << endl;

            // Invia a tutti gli iscritti a quel topic, TRANNE chi invia
            ws->publish(socketData->room, message, opCode, false);
          }
        } catch (...) {}
    }});

  app.listen(9999, [](auto *listen_socket) {
    if (listen_socket) {
      cout << "Server HTTPS/WSS in ascolto su https://localhost:9999" << endl;
    }
  });
  app.run();
  return 0;
}
