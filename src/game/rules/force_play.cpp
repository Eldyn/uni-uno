#include <common/game/gamerule.hpp>
#include <game/rule_registry.hpp>

namespace game {
    class ForcePlayRule : public GameRule {
    public:
        void OnCardDrawn(GameState* state, CardDrawnEvent& event) override {
            event.force_play = true; // Tell the DrawCard engine to bypass DecideDrawnCardEffect
        }
    };

    static RuleRegistrar registrar("force_play", []() { 
        return std::make_unique<ForcePlayRule>(); 
    });
}
