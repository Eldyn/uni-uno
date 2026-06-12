#pragma once
#include <map>
#include <string>
#include <string_view>
#include <App.h>
#include <nlohmann/json.hpp>
#include <database.hpp>
#include <action_router.hpp>
#include <http_router.hpp>
#include <websocket_context.hpp>

/**
 * @file webserver.hpp
 * @brief Definizione della classe principale WebServer che gestisce l'intero ciclo di vita dell'applicazione di rete.
 *
 * Questo file contiene l'infrastruttura base per l'avvio del server HTTP e WebSocket
 * basato su uWebSockets. Gestisce le rotte, l'inizializzazione del database e le connessioni attive.
 */

/**
 * @class WebServer
 * @brief Motore principale dell'applicazione backend.
 * * Inizializza l'applicazione SSL (se configurata), configura i router HTTP e WebSocket,
 * mantiene un registro delle connessioni attive e fornisce hook per eventi di apertura/chiusura connessione.
 * Non supporta copia o assegnamento (classe non copiabile).
 * * @tag SRV-CORE-000
 */
class WebServer {
public:
    /**
     * @brief Costruttore del WebServer.
     * @param port La porta su cui il server starà in ascolto (es. 443 o 8080).
     * @param keyFile Percorso al file della chiave privata SSL.
     * @param certFile Percorso al file del certificato SSL.
     * @param dbFile Percorso al file del database SQLite.
     * @tag SRV-CORE-001
     */
    explicit WebServer(int port,
                       std::string_view keyFile  = "key.pem",
                       std::string_view certFile = "cert.pem",
                       std::string_view dbFile   = "game.db");

    /**
     * @brief Distruttore. Si occupa di rilasciare eventuali risorse pendenti.
     * @tag SRV-CORE-002
     */
    ~WebServer();

    WebServer(const WebServer&)            = delete;
    WebServer& operator=(const WebServer&) = delete;

    /**
     * @brief Avvia il loop di ascolto del server (metodo bloccante).
     * Mette in ascolto uWebSockets sulla porta specificata.
     * @tag SRV-CORE-003
     */
    void Run();

    /**
     * @brief Recupera un riferimento al router delle azioni WebSocket.
     * @return ActionRouter& Router per registrare gli handler dei messaggi WS.
     * @tag SRV-CORE-004
     */
    ActionRouter& GetActionRouter()   { return ws_router_;   }

    /**
     * @brief Recupera un riferimento al router delle richieste HTTP.
     * @return HttpRouter& Router per registrare le rotte REST/HTTP.
     * @tag SRV-CORE-005
     */
    HttpRouter&   GetHTTPRouter()     { return http_router_; }

    /**
     * @brief Recupera l'istanza sottostante dell'applicazione uWebSockets.
     * @return uWS::SSLApp& L'applicazione uWS.
     * @tag SRV-CORE-006
     */
    uWS::SSLApp&  GetApp()            { return app_;         }

    /**
     * @typedef ConnectionHandler
     * @brief Callback invocata quando un WebSocket viene aperto o chiuso.
     * @tag SRV-TYP-001
     */
    using ConnectionHandler = std::function<void(AppWebSocket*, PerSocketData*)>;

    /**
     * @brief Aggiunge una callback da eseguire all'apertura di una nuova connessione WS.
     * @param handler La funzione da registrare.
     * @tag SRV-CORE-007
     */
    void OnConnectionOpen(ConnectionHandler handler);

    /**
     * @brief Aggiunge una callback da eseguire alla chiusura di una connessione WS.
     * @param handler La funzione da registrare.
     * @tag SRV-CORE-008
     */
    void OnConnectionClose(ConnectionHandler handler);

private:
    int    port_;           /**< Porta di ascolto del server. */
    string db_file_;        /**< Percorso del file DB sqlite. */

    uWS::SSLApp app_;       /**< Istanza principale di uWebSockets (SSL/TLS). */
    std::map<string, AppWebSocket*> connections_; /**< Mappa degli utenti connessi (Username -> Socket). */

    ActionRouter ws_router_;    /**< Gestore del dispatching dei messaggi WebSocket. */
    HttpRouter   http_router_;  /**< Gestore del dispatching delle richieste HTTP. */

    /**
     * @brief Inizializza la connessione al database e ne applica lo schema se necessario.
     * @return true se l'inizializzazione ha avuto successo, false altrimenti.
     * @tag SRV-PRIV-001
     */
    bool InitDB();

    /**
     * @brief Registra le rotte interne, applica i wildcard (middleware) all'app uWS.
     * @tag SRV-PRIV-002
     */
    void RegisterRoutes();

    /**
     * @brief Handler generico per richieste HTTP POST (delegeto all'HttpRouter).
     * @tag SRV-PRIV-003
     */
    void HandlePost(AppResponse*, AppRequest*);

    /**
     * @brief Handler generico per richieste HTTP GET (delegato all'HttpRouter).
     * @tag SRV-PRIV-004
     */
    void HandleGet (AppResponse*, AppRequest*);

    /**
     * @brief Gestisce l'evento di upgrade WebSocket andato a buon fine.
     * @tag SRV-PRIV-005
     */
    void OnSocketOpen   (AppWebSocket*);

    /**
     * @brief Riceve i frame WebSocket in ingresso e li inoltra all'ActionRouter.
     * @tag SRV-PRIV-006
     */
    void OnSocketMessage(AppWebSocket*, std::string_view, uWS::OpCode);

    /**
     * @brief Gestisce l'evento di disconnessione di un socket.
     * @tag SRV-PRIV-007
     */
    void OnSocketClosed (AppWebSocket*);

    /**
     * @brief Utilità interna per leggere il contenuto completo di un file (es. per risorse statiche).
     * @param path Percorso del file.
     * @return std::string Il contenuto del file.
     * @tag SRV-UTIL-001
     */
    static std::string     ReadFile    (std::string_view path);

    /**
     * @brief Deduce il MimeType di un file basandosi sull'estensione.
     * @param path Percorso del file.
     * @return std::string Il MIME Type (es. "text/html").
     * @tag SRV-UTIL-002
     */
    static std::string     GetMimeType (const string& path);

    std::vector<ConnectionHandler> on_open_hooks_;   /**< Lista di hook di apertura connessione. */
    std::vector<ConnectionHandler> on_close_hooks_;  /**< Lista di hook di chiusura connessione. */
};
