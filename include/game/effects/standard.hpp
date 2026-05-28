#pragma once

#include <common/game/effect.hpp>
#include <string>
#include <cstdint>


namespace game {

    class AdvanceTurnEffect : public Effect {
    public:
    EffectResult Resolve(GameState* game_state, MatchInstance* match_instance) override;
    };

    class DecideDrawnCardEffect : public Effect {
    public:
        explicit DecideDrawnCardEffect(const std::string& username, uint16_t card_id) : username_(username), card_id_(card_id) {}
            
        EffectResult Resolve(GameState* state, MatchInstance* match) override;
        
    private:
        std::string username_;
        uint16_t card_id_;
    };

    class DrawEffect : public Effect {
    public:
        explicit DrawEffect(int count, const std::string& target_username) : count_(count), target_username_(target_username) {} 

        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;
    
    private:
        int count_;
        std::string target_username_;
    };
    
    class SkipEffect : public Effect {
    public:
        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;
    };
    
    class ReverseEffect : public Effect {
    public:
        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;
    };
    
    class ChooseColorEffect : public Effect {
    public:
        explicit ChooseColorEffect(const std::string& target_username) : target_username_(target_username) {}

        EffectResult Resolve(GameState* state, MatchInstance* match_instance) override;
    
    private:
        std::string target_username_;
    };
}
