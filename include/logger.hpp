#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @file logger.hpp
 * @brief Sistema di logging thread-safe e formattato a colori per la console.
 */

/**
 * @struct Logger
 * @brief Fornisce metodi statici per stampare messaggi di log formattati sullo standard output.
 * * Implementa il supporto per i colori ANSI cross-platform (abilitando il VT processing su Windows)
 * e formatta automaticamente i messaggi con timestamp al millisecondo e tag allineati.
 * @tag LOG-CLS-001
 */
struct Logger {
private:
    // ANSI Color Codes
    static constexpr std::string_view Reset   = "\033[0m";
    static constexpr std::string_view Gray    = "\033[90m";
    static constexpr std::string_view Red     = "\033[31m";
    static constexpr std::string_view Green   = "\033[32m";
    static constexpr std::string_view Yellow  = "\033[33m";
    static constexpr std::string_view Blue    = "\033[34m";
    static constexpr std::string_view Magenta = "\033[35m";
    static constexpr std::string_view Cyan    = "\033[36m";
    static constexpr std::string_view White   = "\033[37m";
    static constexpr std::string_view Bold    = "\033[1m";

    inline static bool is_initialized = false;

    /**
     * @brief Abilita il supporto ANSI sui terminali Windows.
     * @tag LOG-PRIV-001
     */
    static void Init() {
        if (is_initialized) return;
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
#endif
        is_initialized = true;
    }

    /**
     * @brief Genera un timestamp formattato con millisecondi (es. "[14:32:01.045]").
     * @return std::string Timestamp colorato in grigio.
     * @tag LOG-PRIV-002
     */
    static std::string timestamp() {
        auto now = std::chrono::system_clock::now();
        auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now.time_since_epoch()) % 1000;

        time_t t = std::chrono::system_clock::to_time_t(now);
        tm tm_buf;
        #ifdef _WIN32
            localtime_s(&tm_buf, &t);   // In windows gli argomenti sono swappati
        #else
            localtime_r(&t, &tm_buf);   // POSIX
        #endif

        char buf[20];
        strftime(buf, sizeof(buf), "%H:%M:%S", &tm_buf);
        
        std::ostringstream oss;
        oss << Gray << "[" << buf << "." << std::setfill('0') << std::setw(3) << ms.count() << "]" << Reset;
        return oss.str();
    }

    /**
     * @brief Funzione core per la formattazione e la stampa.
     * Utilizza le fold expressions del C++17 per spacchettare in modo efficiente gli argomenti variadici.
     * @tag LOG-PRIV-003
     */
    template<typename... Args>
    static void Print(std::string_view color, std::string_view tag, Args&&... args) {
        if (!is_initialized) Init();

        std::ostringstream oss;
        oss << timestamp() << " ";
        
        oss << Gray << "[" << Reset << color << Bold << std::left << std::setw(5) << tag << Reset << Gray << "] " << Reset;
        
        // C++17 Fold Expression to unpack arguments efficiently
        (oss << ... << std::forward<Args>(args));

        std::cout << oss.str() << "\n";
    }

public:
    /**
     * @brief Registra un messaggio di informazione generico (Verde).
     * @tag LOG-MTH-001
     */
    template<typename... Args>
    static void Info(Args&&... args) { Print(Green, "INFO", std::forward<Args>(args)...); }

    /**
     * @brief Registra un avviso non bloccante (Giallo).
     * @tag LOG-MTH-002
     */
    template<typename... Args>
    static void Warn(Args&&... args) { Print(Yellow, "WARN", std::forward<Args>(args)...); }

    /**
     * @brief Registra un errore critico (Rosso).
     * @tag LOG-MTH-003
     */
    template<typename... Args>
    static void Error(Args&&... args) { Print(Red, "ERROR", std::forward<Args>(args)...); }

    /**
     * @brief Registra un evento specifico del layer WebSocket (Ciano).
     * @tag LOG-MTH-004
     */
    template<typename... Args>
    static void WS(Args&&... args) { Print(Cyan, " WS ", std::forward<Args>(args)...); }

    /**
     * @brief Registra un evento specifico del LobbyController (Giallo scuro).
     * @tag LOG-MTH-005
     */
    template<typename... Args>
    static void Lobby(Args&&... args) { Print(Yellow, "LOBBY", std::forward<Args>(args)...); }

    /**
     * @brief Registra un evento specifico delle richieste HTTP (Magenta).
     * @tag LOG-MTH-006
     */
    template<typename... Args>
    static void HTTP(Args&&... args) { Print(Magenta, "HTTP", std::forward<Args>(args)...); }
    
    /**
     * @brief Registra un log di debug base (Bianco).
     * @tag LOG-MTH-007
     */
    template<typename... Args>
    static void Log(Args&&... args) { Print(White, " LOG", std::forward<Args>(args)...); }

    static constexpr std::string_view ColorWhite() { return White; }
    static constexpr std::string_view ColorReset() { return Reset; }
};
