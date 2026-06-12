#pragma once
#include <string>

using std::string;

/**
 * @file error.hpp
 * @brief Definizione degli errori standard di sistema e protocollo.
 */

/**
 * @struct Error
 * @brief Incapsula le informazioni riguardanti un errore verificatosi nel sistema.
 * * Contiene un codice standard interno ed un messaggio descrittivo per un facile 
 * instradamento e risposta verso il client.
 * @tag ERR-STR-001
 */
struct Error {
    
    /**
     * @enum Code
     * @brief Codici generici che classificano la tipologia di errore.
     * @tag ERR-ENUM-001
     */
    enum class Code {
        kBadRequest,        /**< Richiesta malformata. */
        kInvalidInput,      /**< Dati in ingresso non validi (es. Type mismatch in JSON). */
        kNotFound,          /**< Risorsa non trovata (es. file o lobby inesistente). */
        kUnauthorised,      /**< Mancanza di permessi o token JWT non valido. */
        kConflict,          /**< Conflitto nello stato delle risorse (es. utente già presente). */
        kDatabaseFailure,   /**< Errore interno al driver SQLite. */
        kInternalError,     /**< Eccezione non gestita o errore di logica del server. */
    };

    Code   code;            /**< Tipologia dell'errore. */
    string message;         /**< Dettaglio testuale umano/leggibile. */

    // --- Factory Methods ---

    /** @brief Crea un errore di tipo kBadRequest. @tag ERR-MTH-001 */
    static Error BadRequest   (const string& msg) { return {Code::kBadRequest,       msg}; }
    /** @brief Crea un errore di tipo kInvalidInput. @tag ERR-MTH-002 */
    static Error InvalidInput (const string& msg) { return {Code::kInvalidInput,     msg}; }
    /** @brief Crea un errore di tipo kNotFound. @tag ERR-MTH-003 */
    static Error NotFound     (const string& msg) { return {Code::kNotFound,         msg}; }
    /** @brief Crea un errore di tipo kUnauthorised. @tag ERR-MTH-004 */
    static Error Unauthorised (const string& msg) { return {Code::kUnauthorised,     msg}; }
    /** @brief Crea un errore di tipo kConflict. @tag ERR-MTH-005 */
    static Error Conflict     (const string& msg) { return {Code::kConflict,         msg}; }
    /** @brief Crea un errore di DB kDatabaseFailure. @tag ERR-MTH-006 */
    static Error DatabaseFail (const string& msg) { return {Code::kDatabaseFailure,  msg}; }
    /** @brief Crea un errore generico kInternalError. @tag ERR-MTH-007 */
    static Error Internal     (const string& msg) { return {Code::kInternalError,    msg}; }

    /**
     * @brief Mappa il codice di errore interno al corrispondente Status Code HTTP.
     * Utile per rispondere automaticamente al client in modo standard (es. nelle REST API).
     * @return std::string Status code formattato (es. "404 Not Found").
     * @tag ERR-MTH-008
     */
    std::string HttpStatus() const {
        switch (code) {
            case Code::kBadRequest:      return "400 Bad Request";
            case Code::kInvalidInput:    return "422 Unprocessable Entity";
            case Code::kNotFound:        return "404 Not Found";
            case Code::kUnauthorised:    return "401 Unauthorized";
            case Code::kConflict:        return "409 Conflict";
            case Code::kDatabaseFailure: return "500 Internal Server Error";
            default:                     return "500 Internal Server Error";
        }
    }
};
