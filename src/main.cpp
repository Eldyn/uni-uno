#include <iostream>
#include <nlohmann/json.hpp>
#include <ostream>
#include <string>
#include <uWebSockets/src/App.h>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

std::string readFile(std::string_view path) {
    std::ifstream is(path.data(), std::ios::binary);
    if (!is) return "";
    std::stringstream buffer;
    buffer << is.rdbuf();
    return buffer.str();
}

std::string getMimeType(std::string path) {
    if (path.ends_with(".html")) return "text/html";
    if (path.ends_with(".js"))   return "text/javascript";
    if (path.ends_with(".css"))  return "text/css";
    if (path.ends_with(".svg"))  return "image/svg+xml";
    if (path.ends_with(".png"))  return "image/png";
    return "application/octet-stream";
}
using json = nlohmann::json;

// Ogni Client avrà sta roba
struct PerSocketData {
  int user_id;
  std::string username;
};

int main() {
  uWS::SocketContextOptions ssl_options;
  ssl_options.key_file_name = "key.pem";
  ssl_options.cert_file_name = "cert.pem";

  const int PORT = 9999;

  uWS::SSLApp(ssl_options)
    .get("/*", [](uWS::HttpResponse<true> *response, uWS::HttpRequest *request) {
      response->onAborted([]() { /* Gestione cleanup */ });

      std::string url = std::string(request->getUrl());
      std::string relativePath = (url == "/") ? "index.html" : url.substr(1);

      fs::path baseDir = fs::current_path() / "public";
      fs::path filePath = baseDir / relativePath;


      if (std::filesystem::exists(filePath) && !std::filesystem::is_directory(filePath)) {
        std::string pathStr = filePath.string();
        std::string_view mime = getMimeType(pathStr);

        response->writeHeader("Content-Type", std::string(mime))
           ->writeHeader("X-Content-Type-Options", "nosniff")
           ->end(readFile(pathStr));
      } else {
        std::cout << "Tried finding: " << filePath << " but 404!" << std::endl;
        // Fallback su index.html per gestire il routing lato client (Svelte)
        response->writeStatus("404 Not Found")->end("File non trovato");
      }
    })
    .ws<int>("/*", {
    .open = [](auto *ws) {
      std::cout << "Client connesso in WSS (Secure)!" << std::endl;
    },
    .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
      ws->send(message, opCode);
    }})
    .listen(9999, [](auto *listen_socket) {
      if (listen_socket) {
        std::cout << "Server HTTPS/WSS in ascolto su https://localhost:9999" << std::endl;
      }
    })
  .run();
  return 0;
}
