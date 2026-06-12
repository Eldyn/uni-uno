#include <common/game/gamerule.hpp>
#include <game/rule_registry.hpp>

namespace game {
    class ProgressiveRule : public GameRule {
    public:
        void OnCardDrawn(GameState* state, CardDrawnEvent& event) override {
            event.keep_drawing = true; // Tell the DrawCard engine to loop
        }
    };

    static RuleRegistrar registrar("progressive", []() { 
        return std::make_unique<ProgressiveRule>(); 
    });
}
