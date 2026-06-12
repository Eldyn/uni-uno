#pragma once
#include "common/game/gamerule.hpp"
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include "logger.hpp"

/**
 * @file rule_registry.hpp
 * @brief Gestione della registrazione e istanziazione dinamica delle regole di gioco.
 * * Questo file definisce un sistema basato su Factory Pattern per permettere la 
 * creazione di regole (GameRule) a runtime partendo dal loro nome testuale (es. dal JSON).
 * Utilizza il meccanismo di auto-registrazione statica per disaccoppiare l'aggiunta 
 * di nuove regole dal codice core del motore.
 */

namespace game {

    /**
     * @typedef RuleFactory
     * @brief Definisce il tipo di funzione capace di generare una nuova istanza di una regola.
     * @tag RULE-REG-TYP-001
     */
    using RuleFactory = std::function<std::unique_ptr<GameRule>()>;

    /**
     * @class RuleRegistry
     * @brief Registro globale (Singleton) delle Factory delle regole di gioco.
     * * Mantiene una mappa che associa una stringa identificativa (il nome della mod/regola)
     * alla funzione `RuleFactory` in grado di allocare la classe specifica.
     * @tag RULE-REG-CLS-001
     */
    class RuleRegistry {
    public:
        /**
         * @brief Fornisce l'accesso alla mappa globale delle regole registrate.
         * Utilizza il pattern "Meyers' Singleton" per garantire che la mappa venga 
         * inizializzata in modo thread-safe al primo utilizzo.
         * @return std::unordered_map<std::string, RuleFactory>& Riferimento alla mappa delle regole.
         * @tag RULE-REG-MTH-001
         */
        static std::unordered_map<std::string, RuleFactory>& GetMap() {
            static std::unordered_map<std::string, RuleFactory> registry;
            return registry;
        }

        /**
         * @brief Istanzia una nuova regola di gioco a partire dal suo nome.
         * * Cerca il nome della regola nella mappa e, se trovata, invoca la sua Factory 
         * per restituirne un'istanza allocata in memoria dinamica.
         * @param name L'identificativo testuale della regola (es. "seven_zero").
         * @return std::unique_ptr<GameRule> Puntatore univoco alla nuova istanza, oppure `nullptr` se la regola non esiste.
         * @tag RULE-REG-MTH-002
         */
        static std::unique_ptr<GameRule> Create(const std::string& name) {
            auto& map = GetMap();
            if (map.find(name) != map.end()) {
                return map[name]();
            }
            Logger::Error("[RuleRegistry] Failed to find rule: ", name);
            return nullptr;
        }
    };

    /**
     * @struct RuleRegistrar
     * @brief Struttura di supporto per l'auto-registrazione delle regole.
     * * Questa struttura è concepita per essere istanziata globalmente/staticamente nei 
     * file sorgente (.cpp) delle regole stesse. Al momento dell'inizializzazione del programma,
     * il costruttore viene eseguito e inietta la regola nel `RuleRegistry`.
     * @tag RULE-REG-STR-001
     */
    struct RuleRegistrar {
        /**
         * @brief Costruttore che esegue l'effettiva registrazione della Factory.
         * @param name Il nome con cui la regola sarà conosciuta nel sistema.
         * @param factory La funzione in grado di istanziare la regola.
         * @tag RULE-REG-MTH-003
         */
    RuleRegistrar(const std::string& name, RuleFactory factory) {
            RuleRegistry::GetMap()[name] = std::move(factory);
        }
    };
    
}
