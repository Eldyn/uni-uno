#pragma once
#include <sqlite3.h>
#include <stdexcept>
#include <variant>
#include <vector>
#include <optional>
#include <unordered_map>
#include <result.hpp>

using DbValue = std::variant<int, double, std::string, std::nullptr_t>;

class DbRow {
public:
    void Set(const std::string& col, DbValue val);

    // BUG: Should be Optional instead of throw
    template<typename T>
    T Get(const std::string& col) const {
        if (!Has(col)) {
            throw std::runtime_error("Column not found: " + col);
        }
        return std::get<T>(data_.at(col));
    };

    template<typename T>
    T GetOr(const std::string& col, T fallback) const {
        if (!Has(col) || std::holds_alternative<std::nullptr_t>(data_.at(col))) {
            return fallback;
        }
        return std::get<T>(data_.at(col));
    };

    bool Has(const std::string& col) const;

private:
    std::unordered_map<std::string, DbValue> data_;
};

class Database {
public:
    static Database& Get();   // singleton

    VoidResult Open(std::string_view path);
    VoidResult ApplySchema(const char* sql);
    void       Close();
    bool       IsOpen() const;

    // All queries return Result — never throw
    VoidResult                          Exec     (const char* sql, std::vector<DbValue> params = {});
    Result<std::vector<DbRow>>          Query    (const char* sql, std::vector<DbValue> params = {});
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
