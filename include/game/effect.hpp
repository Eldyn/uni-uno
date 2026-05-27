#pragma once

#include <game/game_state.hpp>
#include <string>

namespace game {

    class Effect {
    public:
        virtual ~Effect() = default;
        virtual EffectResult Resolve(GameState* state) = 0;
    };
    
    // --- Concrete Standard Effects ---
    
    class DrawEffect : public Effect {
    public:
        explicit DrawEffect(int count, const std::string& target_username) : count_(count), target_username_(target_username) {}
    
        EffectResult Resolve(GameState* state) override;
    
    private:
        int count_;
        std::string target_username_;
    };
    
    class SkipEffect : public Effect {
    public:
        EffectResult Resolve(GameState* state) override;
    };
    
    class ReverseEffect : public Effect {
    public:
        EffectResult Resolve(GameState* state) override;
    };
    
    class ChooseColorEffect : public Effect {
    public:
        explicit ChooseColorEffect(const std::string& target_username) : target_username_(target_username) {}
    
        EffectResult Resolve(GameState* state) override;
    
    private:
        std::string target_username_;
    };
    
}
