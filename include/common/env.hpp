#pragma once
#include <string>
#include <string_view>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include "logger.hpp"

//
//  File format:
//    KEY=VALUE          plain assignment
//    KEY="VALUE"        surrounding double-quotes are stripped
//    # comment          lines whose first non-whitespace char is '#'
//    (blank lines)      silently skipped
//
//  All values are pushed into the real process environment via setenv /
//  _putenv_s so that child processes and third-party libraries that call
//  std::getenv() directly also see them.
//
//  Usage:
//    Env::Load(".env");                       // once, at program start
//    std::string s = Env::Require("JWT_SECRET");  // throws if absent
//    std::string s = Env::Get("PORT", "9999");    // fallback if absent
// ---------------------------------------------------------------------------
namespace Env {

//  Returns the named environment variable or throws std::runtime_error.
//  Use for values that must be present for the server to function correctly
//  (e.g. JWT_SECRET) — failing fast here is better than a cryptic failure
//  deeper in the call stack.
inline std::string Require(const std::string& key) {
    const char* val = std::getenv(key.c_str());
    if (!val) {
        throw std::runtime_error("[Env] Required variable '" + key + "' is not set");
    }
    return val;
}

// ---------------------------------------------------------------------------
//  Get
// ---------------------------------------------------------------------------
//
//  Returns the named environment variable, or `fallback` if it is absent.
//  Safe to call before Load() — std::getenv works on variables set by the
//  shell or the OS regardless of whether Load() has run.

inline std::string Get(const std::string& key,
                        const std::string& fallback = "") {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : fallback;
}

// Internal helpers (not intended for direct use by callers)
//
// Cross-platform setenv.
// POSIX: setenv(key, value, overwrite=1) — the '1' means "replace if exists".
// Windows: _putenv_s has no overwrite flag; it always replaces.
inline void SetEnv(const std::string& key, const std::string& value) {
#ifdef _WIN32
    _putenv_s(key.c_str(), value.c_str());
#else
    ::setenv(key.c_str(), value.c_str(), 1);
#endif
}

// Strip leading and trailing ASCII whitespace (' ', '\t', '\r', '\n').
// find_first_not_of returns npos when the entire string is whitespace,
// which the guard converts to an empty return rather than UB.
inline std::string Trim(const std::string& s) {
    constexpr std::string_view kWs = " \t\r\n";
    auto start = s.find_first_not_of(kWs);
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(kWs);
    return s.substr(start, end - start + 1);
}

//  Reads `path` line by line and pushes each KEY=VALUE pair into the process
//  environment. Returns the number of variables successfully loaded.
//
//  Non-fatal if the file doesn't exist: production environments often inject
//  variables directly (Docker, systemd, CI) without a .env file on disk.
inline int Load(std::string_view path = ".env") {
    std::ifstream file(path.data());
    if (!file.is_open()) {
        Logger::Warn("[Env] .env not found at '" + std::string(path) +
                     "' — relying on existing environment variables");
        return 0;
    }

    int count = 0;
    std::string line;

    // std::getline extracts characters until '\n', discarding the delimiter.
    while (std::getline(file, line)) {
        // Skip blank lines and comment lines (first non-ws char is '#').
        if (line.empty() || line[0] == '#') continue;

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;   // no '=' → malformed, skip

        std::string key   = Trim(line.substr(0, eq));
        std::string value = Trim(line.substr(eq + 1));

        // Strip optional surrounding double-quotes: "value" → value
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }

        if (key.empty()) continue;

        SetEnv(key, value);
        ++count;
    }

    Logger::Info("[Env] Loaded " + std::to_string(count) +
                 " variable(s) from '" + std::string(path) + "'");
    return count;
}

}
