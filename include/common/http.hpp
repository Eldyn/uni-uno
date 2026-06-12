#pragma once
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <websocket_context.hpp>

/**
 * @file http.hpp
 * @brief Funzioni di utilità per la gestione del protocollo HTTP.
 * * Poiché uWebSockets utilizza un'architettura non-bloccante event-driven,
 * questo file contiene helper cruciali per semplificare l'estrazione in sicurezza 
 * di body chunked e il parsing degli header (es. Cookies).
 */

namespace http {

/**
 * @brief Accumula asincronamente un intero body HTTP frammentato (chunked) da uWebSockets.
 * * Poiché uWebSockets non è bloccante, i body arrivano in uno o più frammenti `string_view`
 * tramite la callback `onData`. Il parametro booleano `isLast` segnala l'ultimo frammento.
 * Questa funzione bufferizza i frammenti autonomamente e agisce solo a trasferimento completato.
 * * @details Implementa una guardia di memoria `is_alive` (`std::shared_ptr<bool>`) per gestire le disconnessioni
 * improvvise. Se il client interrompe il trasferimento, uWS lancia `onAborted`. La guardia diventa `false`
 * evitando che successivi trigger di `onData` accedano al puntatore `res` divenuto pendente (dangling).
 * * @param res Puntatore all'oggetto Response della richiesta HTTP corrente. Deve restare valido fino al termine.
 * @param max_bytes Limite massimo rigido in byte (Hard cap) per evitare attacchi DDoS. Se superato, restituisce 413.
 * @param callback La funzione da invocare, una volta sola, con la stringa completa del body (se non abortita).
 * @tag CMN-HTTP-MTH-001
 */
inline void ReadBody(AppResponse* res, size_t max_bytes, std::function<void(const std::string&)> callback) {
    auto is_alive = std::make_shared<bool>(true);
    auto buffer   = std::make_shared<std::string>();

    res->onAborted([is_alive] { *is_alive = false; });

    res->onData([is_alive, buffer, max_bytes, callback = std::move(callback), res](std::string_view chunk, bool isLast) mutable {
        if (!*is_alive) return;

        buffer->append(chunk.data(), chunk.size());

        if (buffer->size() > max_bytes) {
            res->writeStatus("413 Payload Too Large")->end();
            return;
        }

        if (isLast) callback(*buffer);
    });
}

/**
 * @brief Rimuove gli spazi bianchi ASCII (spazi, tabulazioni, a capo) all'inizio e alla fine di una stringa.
 * Essendo `constexpr`, se valutata a tempo di compilazione non genera overhead.
 * * @param str La vista della stringa originale.
 * @return std::string_view Una porzione centrata della stringa priva degli spazi esterni. Restituisce una vista vuota se composta solo da spazi.
 * @tag CMN-HTTP-MTH-002
 */
constexpr std::string_view TrimWhitespace(std::string_view str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        return {};
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

/**
 * @brief Estrae in modo sicuro il valore di un singolo cookie dalla stringa raw dell'header.
 * * Scansiona iterativamente la stringa dell'header `Cookie` (es. `session=123; user=abc`), 
 * separando le coppie al punto e virgola e cercando la corrispondenza esatta della chiave,
 * ripulendo eventuali spazi generati dal browser.
 * * @param cookie_header La stringa nativa dell'header HTTP contenente i cookie.
 * @param target_key La chiave identificativa del cookie da ricercare.
 * @return std::optional<std::string> Il valore del cookie decodificato, oppure una stringa vuota se la chiave non esiste.
 * @tag CMN-HTTP-MTH-003
 */
inline std::optional<std::string> GetCookieValue(std::string_view cookie_header, std::string_view target_key) {
    size_t start = 0;
    
    while (start < cookie_header.size()) {
        size_t end = cookie_header.find(';', start);
        
        std::string_view pair = (end == std::string_view::npos) 
                                ? cookie_header.substr(start) 
                                : cookie_header.substr(start, end - start);

        // Separa la coppia in Chiave e Valore (Key/Value)
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string_view::npos) {
            std::string_view key = TrimWhitespace(pair.substr(0, eq_pos));
            
            if (key == target_key) {
                return std::string(TrimWhitespace(pair.substr(eq_pos + 1)));
            }
        }

        if (end == std::string_view::npos) {
            break;
        }
        
        start = end + 1;
    }

    return "";
}

}
