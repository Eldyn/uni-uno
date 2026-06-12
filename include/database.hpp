#pragma once
#include "logger.hpp"
#include <sqlite3.h>
#include <stdexcept>
#include <variant>
#include <vector>
#include <optional>
#include <unordered_map>
#include <result.hpp>

/**
 * @file database.hpp
 * @brief Astrazione leggera e sicura (Type-Safe) attorno alla libreria SQLite3.
 * * Fornisce accesso Singleton al database, con un sistema di query basato 
 * su `std::variant` per prevenire SQL Injection, ed un costrutto RAII per le transazioni.
 */

/**
 * @typedef DbValue
 * @brief Variante che incapsula i tipi di dati base ammessi nel database SQLite.
 * @tag DB-TYP-001
 */
using DbValue = std::variant<int, double, std::string, std::nullptr_t>;

/**
 * @class DbRow
 * @brief Rappresenta una singola riga restituita da una query SQL.
 * Memorizza i dati usando le chiavi di colonna come indici della mappa interna.
 * @tag DB-ROW-001
 */
class DbRow {
public:
    /**
     * @brief Imposta un valore nella riga per una specifica colonna.
     * @param col Il nome della colonna.
     * @param val Il valore tipizzato.
     * @tag DB-ROW-MTH-001
     */
    void Set(const std::string& col, DbValue val);

    /**
     * @brief Estrae un valore forzandone il tipo previsto (Lancia eccezione se la colonna manca).
     * @tparam T Tipo di dato atteso (es. int, std::string).
     * @param col Nome della colonna.
     * @return T Valore estratto.
     * @throws std::runtime_error se la colonna non esiste.
     * @tag DB-ROW-MTH-002
     */
    template<typename T>
    T Get(const std::string& col) const {
        if (!Has(col)) {
            throw std::runtime_error("Column not found: " + col);
        }
        return std::get<T>(data_.at(col));
    };

    /**
     * @brief Estrae un valore in modo sicuro fornendo un fallback nel caso sia assente o NULL.
     * @tparam T Tipo di dato atteso.
     * @param col Nome della colonna.
     * @param fallback Valore di default in caso di colonna NULL/mancante.
     * @return T Il valore estratto o il fallback.
     * @tag DB-ROW-MTH-003
     */
    template<typename T>
    T GetOr(const std::string& col, T fallback) const {
        if (!Has(col) || std::holds_alternative<std::nullptr_t>(data_.at(col))) {
            return fallback;
        }
        return std::get<T>(data_.at(col));
    };

    /**
     * @brief Verifica se la riga possiede la colonna specificata.
     * @param col Nome della colonna.
     * @return true se presente, false altrimenti.
     * @tag DB-ROW-MTH-004
     */
    bool Has(const std::string& col) const;

private:
    std::unordered_map<std::string, DbValue> data_;
};

/**
 * @class Database
 * @brief Pattern Singleton che incapsula la connessione univoca e condivisa al DB SQLite.
 * * Tutti i metodi di query sono "No-Throw" e ritornano risultati monadici (Result) per una
 * corretta gestione degli errori senza eccezioni C++.
 * @tag DB-CLS-001
 */
class Database {
public:
    /**
     * @brief Recupera l'unica istanza attiva del Database.
     * @return Database& Riferimento al Singleton.
     * @tag DB-MTH-001
     */
    static Database& Get();

    /**
     * @brief Apre o crea il file del database SQLite nel percorso fornito.
     * @param path Percorso su disco.
     * @return VoidResult Risultato del tentativo di apertura.
     * @tag DB-MTH-002
     */
    VoidResult Open(std::string_view path);

    /**
     * @brief Applica comandi SQL diretti al database (utilizzato prevalentemente per i file di Schema).
     * @param sql Stringa query SQL raw.
     * @return VoidResult.
     * @tag DB-MTH-003
     */
    VoidResult ApplySchema(const char* sql);

    /**
     * @brief Chiude esplicitamente la connessione SQLite attiva.
     * @tag DB-MTH-004
     */
    void       Close();

    /**
     * @brief Verifica se il DB è attualmente pronto all'uso.
     * @return true se aperto.
     * @tag DB-MTH-005
     */
    bool       IsOpen() const;

    /**
     * @brief Esegue una query che non restituisce righe (es. INSERT, UPDATE, DELETE).
     * @param sql La stringa SQL (che può contenere segnaposto `?`).
     * @param params I parametri tipizzati opzionali da bindare sulla query.
     * @return VoidResult Errore in caso di fallimento della query o violazione constraint.
     * @tag DB-MTH-006
     */
    VoidResult                          Exec     (const char* sql, std::vector<DbValue> params = {});

    /**
     * @brief Esegue una query che restituisce una lista di risultati (es. SELECT generiche).
     * @param sql Stringa query parametrizzata.
     * @param params Lista di parametri associati ai segnaposto.
     * @return Result<std::vector<DbRow>> Il vettore di righe risultanti.
     * @tag DB-MTH-007
     */
    Result<std::vector<DbRow>>          Query    (const char* sql, std::vector<DbValue> params = {});

    /**
     * @brief Esegue una query mirata alla restituzione di al massimo 1 record (es. SELECT con LIMIT 1).
     * @param sql Stringa query parametrizzata.
     * @param params Lista di parametri.
     * @return Result<std::optional<DbRow>> Risultato contenente il record o vuoto (std::nullopt).
     * @tag DB-MTH-008
     */
    Result<std::optional<DbRow>>        QueryOne (const char* sql, std::vector<DbValue> params = {});

private:
    Database()  = default;
    ~Database() { Close(); }
    Database(const Database&)            = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* db_ = nullptr;

    sqlite3_stmt* Prepare(const char* sql, const std::vector<DbValue>& params);
    DbRow         ReadRow(sqlite3_stmt* stmt);
};

/**
 * @class TransactionGuard
 * @brief Implementa l'esecuzione sicura di transazioni database tramite il paradigma RAII.
 * * Garantisce che un eventuale fallimento bloccante all'interno del suo scope C++ 
 * scateni automaticamente un ROLLBACK (se non è stato chiamato esplicitamente `Commit()`),
 * evitando inconsistenze nel database.
 * @tag DB-TX-001
 */
class TransactionGuard {
public:
    /**
     * @brief Inizia la transazione eseguendo "BEGIN TRANSACTION".
     * @param db Il database su cui operare.
     * @tag DB-TX-MTH-001
     */
    explicit TransactionGuard(Database& db) : db_(db), committed_(false) {
        auto res = db_.Exec("BEGIN TRANSACTION;");
        if (!res) {
            throw std::runtime_error("Failed to begin transaction: " + res.error().message);
        }
    }

    /**
     * @brief Distruttore: Esegue "ROLLBACK" se la transazione non è stata completata regolarmente.
     * @tag DB-TX-MTH-002
     */
    ~TransactionGuard() {
        if (!committed_) {
            auto _ = db_.Exec("ROLLBACK;");
            Logger::Error("[DB] Transaction failed, rolling back...");
        }
    }

    /**
     * @brief Marca la transazione per un completamento vincente (esegue "COMMIT").
     * Se chiamato, annulla l'effetto di rollback alla distruzione del Guard.
     * @tag DB-TX-MTH-003
     */
    void Commit() {
        if (!committed_) {
            auto res = db_.Exec("COMMIT;");
            if (!res) {
                throw std::runtime_error("Failed to COMMIT transaction: " + res.error().message);
            }
            committed_ = true;
        }
    }

    TransactionGuard(const TransactionGuard&) = delete;
    TransactionGuard& operator=(const TransactionGuard&) = delete;

private:
    Database& db_;
    bool committed_;
};
