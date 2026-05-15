#pragma once
#include <string>
#include <result.hpp>
#include <database.hpp>
#include <http_router.hpp>

// ---------------------------------------------------------------------------
//  AuthController
// ---------------------------------------------------------------------------
//
//  Responsibilities:
//    • POST /auth/register  — validate input, hash password, persist user
//    • POST /auth/login     — verify password, issue a signed JWT
//    • VerifyToken()        — validate a JWT and extract the payload;
//                             used by WebServer during WS upgrade
//
//  Password storage strategy:
//    We use PBKDF2-HMAC-SHA256 (via OpenSSL's EVP_PBEKeyIvGen / PKCS5_PBKDF2_HMAC)
//    with a per-user random 16-byte salt and a configurable iteration count.
//    The resulting hash (32 bytes) is stored base64-encoded alongside the salt.
//    A global pepper (from JWT_SECRET env var) is appended to the password
//    before hashing so that a stolen DB alone is not enough to crack passwords.
//
//  JWT strategy:
//    Tokens are HS256-signed using jwt-cpp.
//    Payload carries: sub (username), iat (issued-at), exp (expiry, 24 h).
//    The secret is read from the JWT_SECRET environment variable.
// ---------------------------------------------------------------------------

struct JwtPayload {
    std::string username;
};

class AuthController {
public:
    // Registers HTTP routes on the supplied router.
    // The router must outlive this module.
    explicit AuthController(HttpRouter& router);

    // Verify a JWT string.  Returns the payload on success, or an Error.
    // Called by WebServer on every WebSocket upgrade.
    static Result<JwtPayload> VerifyToken(const std::string& token);

private:
    // ---- HTTP handlers ----
    void HandleRegister(AppResponse* res, AppRequest* req);
    void HandleLogin   (AppResponse* res, AppRequest* req);

    // ---- Password helpers ----
    // Returns "<base64_salt>:<base64_hash>"
    static std::string HashPassword (const std::string& password);
    // Returns true if `password` matches the stored "<salt>:<hash>" record
    static bool        VerifyPassword(const std::string& password,
                                      const std::string& stored);

    // ---- JWT helpers ----
    static std::string IssueToken(const std::string& username);

    // ---- Misc ----
    // Read the full HTTP body then call `callback(body)`.
    // Handles the async uWS onData pattern uniformly.
    static void ReadBody(AppResponse* res, std::function<void(const std::string&)> callback);

    // Minimum acceptable lengths to reject trivially bad inputs early.
    static constexpr int kMinUsernameLen = 3;
    static constexpr int kMaxUsernameLen = 32;
    static constexpr int kMinPasswordLen = 8;
    static constexpr int kMaxBodyBytes   = 4096;

    // PBKDF2 parameters — increasing kIterations raises brute-force cost.
    static constexpr int kSaltBytes   = 16;
    static constexpr int kHashBytes   = 32;
    static constexpr int kIterations  = 200'000;   // NIST SP 800-132 minimum
};
