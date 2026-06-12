#pragma once
#include <string>
#include <result.hpp>
#include <database.hpp>
#include <http_router.hpp>

/**
 * @file auth_controller.hpp
 * @brief Controller per l'autenticazione, registrazione e gestione dei token JWT.
 * * Gestisce l'identità degli utenti nel sistema implementando endpoint HTTP sicuri.
 * Si occupa dell'hashing crittografico delle password e della generazione/validazione
 * dei token Web JSON (JWT) utilizzati per l'autorizzazione WebSocket.
 */

/**
 * @struct JwtPayload
 * @brief Rappresenta i dati (payload) estratti da un token JWT verificato con successo.
 * @tag CTRL-AUTH-STR-001
 */
struct JwtPayload {
    std::string username; /**< Il nome identificativo dell'utente autenticato (campo 'sub' del token). */
};

/**
 * @class AuthController
 * @brief Gestisce le rotte HTTP per il login/registrazione e la sicurezza crittografica.
 * * **Strategia di archiviazione Password:**
 * Utilizza l'algoritmo PBKDF2-HMAC-SHA256 (tramite OpenSSL) con un "salt" casuale di 16-byte
 * per ogni utente e un numero configurabile di iterazioni (200.000, come da standard NIST).
 * L'hash risultante (32 byte) è salvato codificato in base64 insieme al salt. 
 * Inoltre, un "pepper" globale (letto dalla variabile d'ambiente `JWT_SECRET`) viene 
 * appeso alla password prima dell'hashing, rendendo un eventuale furto del solo database 
 * insufficiente per craccare le password.
 * * **Strategia JWT:**
 * I token sono firmati in HS256 utilizzando `jwt-cpp`.
 * Il payload trasporta: `sub` (username), `iat` (issued-at), `exp` (scadenza impostata a 24 ore).
 * @tag CTRL-AUTH-CLS-001
 */
class AuthController {
public:
    /**
     * @brief Costruttore dell'AuthController.
     * Registra in automatico le rotte POST e GET sul router HTTP fornito (`/auth/register`, `/auth/login`, ecc.).
     * * @param router Riferimento al router HTTP centrale. Deve sopravvivere a questa classe.
     * @tag CTRL-AUTH-MTH-001
     */
    explicit AuthController(HttpRouter& router);

    /**
     * @brief Verifica matematicamente un token JWT e ne estrae il payload.
     * Metodo statico richiamato dal `WebServer` all'inizio di ogni tentativo di upgrade
     * verso il protocollo WebSocket, per bloccare client non autorizzati.
     * * @param token La stringa JWT grezza passata dal client (solitamente estratta dai cookie o dall'header).
     * @return Result<JwtPayload> Il payload estratto in caso di successo, oppure un Errore.
     * @tag CTRL-AUTH-MTH-002
     */
    static Result<JwtPayload> VerifyToken(const std::string& token);

private:
    // --- HTTP Route Handlers ---

    /**
     * @brief Gestisce la rotta POST `/auth/register`.
     * Valida gli input (lunghezza e caratteri speciali), calcola l'hash sicuro della password
     * e memorizza il nuovo utente nel database.
     * @param res Puntatore alla risposta HTTP.
     * @param req Puntatore alla richiesta HTTP.
     * @tag CTRL-AUTH-ACT-001
     */
    void HandleRegister(AppResponse* res, AppRequest* req);

    /**
     * @brief Gestisce la rotta POST `/auth/login`.
     * Cerca l'utente nel database, ricalcola l'hash per verificare la password fornita 
     * e, in caso di successo, emette e restituisce un JWT firmato.
     * @param res Puntatore alla risposta HTTP.
     * @param req Puntatore alla richiesta HTTP.
     * @tag CTRL-AUTH-ACT-002
     */
    void HandleLogin   (AppResponse* res, AppRequest* req);

    /**
     * @brief Gestisce la rotta POST `/auth/logout`.
     * Invalida la sessione lato client (es. richiedendo l'eliminazione del cookie JWT).
     * @param res Puntatore alla risposta HTTP.
     * @param req Puntatore alla richiesta HTTP.
     * @tag CTRL-AUTH-ACT-003
     */
    void HandleLogout  (AppResponse* res, AppRequest* req);

    /**
     * @brief Gestisce la rotta GET `/auth/me`.
     * Restituisce i dettagli dell'utente attualmente autenticato valutandone il token.
     * @param res Puntatore alla risposta HTTP.
     * @param req Puntatore alla richiesta HTTP.
     * @tag CTRL-AUTH-ACT-004
     */
    void HandleMe      (AppResponse* res, AppRequest* req);

    // --- Helpers Crittografici ---

    /**
     * @brief Genera un salt crittografico e calcola l'hash della password.
     * @param password La password in chiaro inserita dall'utente.
     * @return std::string Formato composto salvato su DB: `<base64_salt>:<base64_hash>`.
     * @tag CTRL-AUTH-CRYP-001
     */
    static std::string HashPassword (const std::string& password);

    /**
     * @brief Verifica se una password in chiaro corrisponde a quella memorizzata sul DB.
     * @param password La password in chiaro inserita nel login.
     * @param stored La stringa dal DB in formato `<salt>:<hash>`.
     * @return true se le credenziali combaciano, false altrimenti.
     * @tag CTRL-AUTH-CRYP-002
     */
    static bool        VerifyPassword(const std::string& password, const std::string& stored);

    /**
     * @brief Genera un nuovo token JWT firmato usando la chiave segreta (pepper).
     * @param username Il nome utente da inserire nel payload (campo 'sub').
     * @return std::string La stringa del token completa.
     * @tag CTRL-AUTH-CRYP-003
     */
    static std::string IssueToken(const std::string& username);

    // --- Parametri di validazione e Sicurezza ---

    static constexpr int kMinUsernameLen = 3;       /**< @brief Lunghezza minima dell'username. @tag CTRL-AUTH-CFG-001 */
    static constexpr int kMaxUsernameLen = 32;      /**< @brief Lunghezza massima dell'username. @tag CTRL-AUTH-CFG-002 */
    static constexpr int kMinPasswordLen = 8;       /**< @brief Lunghezza minima della password. @tag CTRL-AUTH-CFG-003 */
    static constexpr int kMaxBodyBytes   = 4096;    /**< @brief Limite in bytes per il payload HTTP (Anti-DDoS). @tag CTRL-AUTH-CFG-004 */

    // Parametri PBKDF2 (Aumentare kIterations alza il costo per i tentativi di brute-force)
    
    static constexpr int kSaltBytes   = 16;         /**< @brief Dimensione in bytes del Salt crittografico. @tag CTRL-AUTH-CFG-005 */
    static constexpr int kHashBytes   = 32;         /**< @brief Dimensione in bytes dell'Hash risultante. @tag CTRL-AUTH-CFG-006 */
    static constexpr int kIterations  = 200'000;    /**< @brief Numero di iterazioni dell'algoritmo PBKDF2 (minimo NIST SP 800-132). @tag CTRL-AUTH-CFG-007 */
};
