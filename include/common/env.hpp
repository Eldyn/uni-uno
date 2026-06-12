#pragma once
#include <string>
#include <string_view>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include <logger.hpp>

/**
 * @file env.hpp
 * @brief Gestione delle variabili d'ambiente e caricamento dal file di configurazione locale `.env`.
 * * Formato del file supportato:
 * - `KEY=VALUE` (Assegnamento base)
 * - `KEY="VALUE"` (I doppi apici circostanti vengono rimossi automaticamente)
 * - `# commento` (Le righe che iniziano con '#' vengono ignorate)
 * - Righe vuote saltate silenziosamente
 * * Tutti i valori vengono iniettati nell'ambiente di processo nativo del sistema operativo 
 * (tramite `setenv` o `_putenv_s`), rendendoli visibili anche a processi figli o librerie 
 * di terze parti che chiamano direttamente `std::getenv()`.
 */

namespace Env {

/**
 * @brief Recupera il valore di una variabile d'ambiente obbligatoria.
 * * Da utilizzare per valori fondamentali senza i quali il server non può funzionare 
 * correttamente (es. `JWT_SECRET`). Fallire rapidamente (fail-fast) all'avvio è 
 * preferibile rispetto a un errore criptico generato in profondità nello stack di chiamate.
 * * @param key Il nome della variabile d'ambiente da leggere.
 * @return std::string Il valore estratto dall'ambiente operativo.
 * @throws std::runtime_error Se la variabile richiesta non è definita nell'ambiente.
 * @tag CMN-ENV-MTH-001
 */
inline std::string Require(const std::string& key) {
    const char* val = std::getenv(key.c_str());
    if (!val) {
        throw std::runtime_error("[Env] Required variable '" + key + "' is not set");
    }
    return val;
}

/**
 * @brief Recupera il valore di una variabile d'ambiente, fornendo un fallback opzionale.
 * * Sicuro da chiamare anche prima dell'esecuzione di `Load()`, poiché interagisce 
 * direttamente con le variabili di sistema fornite dalla shell o dal SO.
 * * @param key Il nome della variabile d'ambiente da leggere.
 * @param fallback Il valore di default da restituire se la variabile non esiste.
 * @return std::string Il valore estratto o la stringa di fallback.
 * @tag CMN-ENV-MTH-002
 */
inline std::string Get(const std::string& key,
                        const std::string& fallback = "") {
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : fallback;
}

/**
 * @brief Funzione interna helper per impostare una variabile d'ambiente in modo cross-platform.
 * * POSIX: Utilizza `setenv(key, value, 1)` dove '1' forza la sovrascrittura se esiste già.
 * Windows: Utilizza `_putenv_s` che sovrascrive per comportamento predefinito.
 * * @param key Il nome della variabile da impostare.
 * @param value Il valore da assegnare alla variabile.
 * @tag CMN-ENV-MTH-003
 */
inline void SetEnv(const std::string& key, const std::string& value) {
#ifdef _WIN32
    _putenv_s(key.c_str(), value.c_str());
#else
    ::setenv(key.c_str(), value.c_str(), 1);
#endif
}

/**
 * @brief Funzione interna helper per rimuovere gli spazi bianchi ASCII all'inizio e alla fine di una stringa.
 * Rimuove i caratteri ' ', '\t', '\r', '\n'. Previene comportamenti indefiniti (UB) restituendo 
 * una stringa vuota qualora la stringa d'ingresso sia composta interamente da spazi.
 * * @param s La stringa da ripulire.
 * @return std::string La stringa ripulita ("trimmata").
 * @tag CMN-ENV-MTH-004
 */
inline std::string Trim(const std::string& s) {
    constexpr std::string_view kWs = " \t\r\n";
    auto start = s.find_first_not_of(kWs);
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(kWs);
    return s.substr(start, end - start + 1);
}

/**
 * @brief Legge il file riga per riga e inietta ogni coppia CHIAVE=VALORE nell'ambiente di processo.
 * * Non genera errori fatali se il file non esiste: gli ambienti di produzione moderni (Docker, 
 * systemd, CI) iniettano spesso le variabili d'ambiente in modo diretto senza creare alcun file 
 * `.env` sul disco.
 * * @param path Il percorso del file da leggere (default: ".env").
 * @return int Il numero esatto di variabili d'ambiente lette ed iniettate con successo.
 * @tag CMN-ENV-MTH-005
 */
inline int Load(std::string_view path = ".env") {
    std::ifstream file(path.data());
    if (!file.is_open()) {
        Logger::Warn("[Env] .env not found at '" + std::string(path) +
                     "' — relying on existing environment variables");
        return 0;
    }

    int count = 0;
    std::string line;

    // std::getline estrae i caratteri fino al '\\n', scartando il delimitatore stesso.
    while (std::getline(file, line)) {
        // Ignora le righe vuote e i commenti (primo carattere non vuoto è '#').
        if (line.empty() || line[0] == '#') continue;

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;   // nessuna '=' → malformata, salta

        std::string key   = Trim(line.substr(0, eq));
        std::string value = Trim(line.substr(eq + 1));

        // Rimuove eventuali doppi apici circostanti opzionali: "value" → value
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
