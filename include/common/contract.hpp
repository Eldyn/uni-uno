#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
#include <logger.hpp>

struct ContractConstants {
    int USERNAME_MIN = 3;
    int USERNAME_MAX = 32;
    int PASSWORD_MIN = 8;
    int LOBBY_NAME_MAX = 50;
    int LOBBY_CODE_LEN = 6;
    int MAX_LOBBY_MEMBERS = 10;

    static ContractConstants& Get() {
        static ContractConstants instance;
        return instance;
    }

    void Load(const std::string& path) {
        try {
            std::ifstream f(path);
            if (!f.is_open()) {
                Logger::Error("Could not open contract file: ", path);
                return;
            }
            nlohmann::json j = nlohmann::json::parse(f);
            if (j.contains("USERNAME_MIN")) USERNAME_MIN = j["USERNAME_MIN"];
            if (j.contains("USERNAME_MAX")) USERNAME_MAX = j["USERNAME_MAX"];
            if (j.contains("PASSWORD_MIN")) PASSWORD_MIN = j["PASSWORD_MIN"];
            if (j.contains("LOBBY_NAME_MAX")) LOBBY_NAME_MAX = j["LOBBY_NAME_MAX"];
            if (j.contains("LOBBY_CODE_LEN")) LOBBY_CODE_LEN = j["LOBBY_CODE_LEN"];
            if (j.contains("MAX_LOBBY_MEMBERS")) MAX_LOBBY_MEMBERS = j["MAX_LOBBY_MEMBERS"];
            Logger::Info("Contract constants loaded.");
        } catch (const std::exception& e) {
            Logger::Error("Failed to load contract constants: ", e.what());
        }
    }
};
