#pragma once
#include <expected>
#include <error.hpp>

/**
 * @file result.hpp
 * @brief Definisce alias per la gestione funzionale degli errori tramite std::expected.
 * * Sfrutta la funzionalità C++23 (o backport) `std::expected` per ritornare 
 * risultati in maniera monadica, evitando l'overhead delle eccezioni (try/catch).
 */

/**
 * @typedef Result
 * @brief Rappresenta un'operazione che può restituire un valore di tipo T o un Error.
 * @tparam T Il tipo del dato ritornato in caso di successo.
 * @tag RES-TYP-001
 */
template<typename T>
using Result = std::expected<T, Error>;

/**
 * @typedef VoidResult
 * @brief Rappresenta un'operazione senza tipo di ritorno che può fallire restituendo un Error.
 * Tipicamente usato per query al DB (UPDATE/DELETE) che non ritornano set di dati.
 * @tag RES-TYP-002
 */
using VoidResult = std::expected<void, Error>;
