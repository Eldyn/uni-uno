#pragma once
#include "common/game/gamerule.hpp"
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include "logger.hpp"

/**
 * @file rule_registry.hpp
 * @brief Management of the dynamic registration and instantiation of game rules.
 * * This file defines a system based on the Factory Pattern to allow the
 * creation of rules (GameRule) at runtime starting from their textual name (e.g. from JSON).
 * It uses a static self-registration mechanism to decouple the addition
 * of new rules from the core engine code.
 */

namespace game {

    /**
     * @typedef RuleFactory
     * @brief Defines the type of function capable of generating a new instance of a rule.
     * @tag RULE-REG-TYP-001
     */
    using RuleFactory = std::function<std::unique_ptr<GameRule>()>;

    /**
     * @class RuleRegistry
     * @brief Global (Singleton) registry of the game rule factories.
     * * Maintains a map that associates an identifying string (the name of the mod/rule)
     * with the `RuleFactory` function able to allocate the specific class.
     * @tag RULE-REG-CLS-001
     */
    class RuleRegistry {
    public:
        /**
         * @brief Provides access to the global map of registered rules.
         * Uses the "Meyers' Singleton" pattern to ensure the map is
         * initialized in a thread-safe manner on first use.
         * @return std::unordered_map<std::string, RuleFactory>& Reference to the map of rules.
         * @tag RULE-REG-MTH-001
         */
        static std::unordered_map<std::string, RuleFactory>& GetMap() {
            static std::unordered_map<std::string, RuleFactory> registry;
            return registry;
        }

        /**
         * @brief Instantiates a new game rule from its name.
         * * Looks up the rule name in the map and, if found, invokes its Factory
         * to return an instance allocated in dynamic memory.
         * @param name The textual identifier of the rule (e.g. "seven_zero").
         * @return std::unique_ptr<GameRule> Unique pointer to the new instance, or `nullptr` if the rule does not exist.
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
     * @brief Support structure for the self-registration of rules.
     * * This structure is designed to be instantiated globally/statically in the
     * source files (.cpp) of the rules themselves. At program initialization time,
     * the constructor runs and injects the rule into the `RuleRegistry`.
     * @tag RULE-REG-STR-001
     */
    struct RuleRegistrar {
        /**
         * @brief Constructor that performs the actual registration of the Factory.
         * @param name The name by which the rule will be known in the system.
         * @param factory The function able to instantiate the rule.
         * @tag RULE-REG-MTH-003
         */
    RuleRegistrar(const std::string& name, RuleFactory factory) {
            RuleRegistry::GetMap()[name] = std::move(factory);
        }
    };

}
