#pragma once

#include <common/game/effect.hpp>
#include <string>
#include <cstdint>


namespace game {

    class AdvanceTurnEffect : public Effect {
    public:
        EffectResult Resolve(GameState* game_state, MatchInstance* match_instance) override;
        EffectType GetType() const override { return EffectType::kAdvanceTurn; }
    };

    class DecideDrawnCardEffect : public Effect {
    public:
        explicit DecideDrawnCardEffect(const std::string& username, uint16_t card_id) : username_(username), card_id_(card_id) {}
            
        EffectResult Resolve(GameState* state, MatchInstance* match) override;
        EffectType GetType() const override { return EffectType::kDecideDrawnCard; }
    private:
        std::string username_;
        uint16_t card_id_;
    };

    class DrawEffect : public Effect {
    public:
        explicit DrawEffect(int count, const std::string& target_username) : count_(count), target_username_(target_username) {} 

        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;
        EffectType GetType() const override { return EffectType::kDraw; }
    private:
        int count_;
        std::string target_username_;
    };
    
    class SkipEffect : public Effect {
    public:
        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;
        EffectType GetType() const override { return EffectType::kSkip; }
    };
    
    class ReverseEffect : public Effect {
    public:
        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;
        EffectType GetType() const override { return EffectType::kReverse; }
    };
    
    class ChooseColorEffect : public Effect {
    public:
        explicit ChooseColorEffect(const std::string& target_username) : target_username_(target_username) {}

        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;
        EffectType GetType() const override { return EffectType::kChooseColor; }
    private:
        std::string target_username_;
    };
}
