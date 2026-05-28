#include <game/match_instance.hpp>
#include <game/rules/standard.hpp>
#include <common/game/effect.hpp>
#include <controllers/lobby_controller.hpp>
#include <algorithm>
#include <random>

namespace game {
    void ReshuffleDiscardIntoDraw(GameState* game_state) {
        if (game_state->discard_pile.size() <= 1) {
            return; 
        }
        
        CompactCard top_card = game_state->discard_pile.back();
        game_state->discard_pile.pop_back();
    
        game_state->draw_pile = std::move(game_state->discard_pile);
        
        game_state->discard_pile.push_back(top_card);
    
        std::random_device random_device;
        std::mt19937 random_generator(random_device());
        std::shuffle(game_state->draw_pile.begin(), game_state->draw_pile.end(), random_generator);
    }

    MatchInstance::MatchInstance(const std::vector<std::string>& usernames, LobbySettings& settings) : settings_(settings) {
        for (const auto& uname : usernames) {
            state_.players.push_back({uname, {}, false});
        }
        
        for (const auto& mod_name : settings.active_mods) {
            // e.g., if (mod_name == "seven_swap_mod") {
            //     active_rules_.push_back(std::make_unique<SevenSwapMod>());
            // }
        }
    
        active_rules_.push_back(std::make_unique<StandardRule>());
    }
    
    void MatchInstance::Start() {
        GenerateDeck();
        
        // Deal 7 cards to everyone
        for (auto& player : state_.players) {
            for (int i = 0; i < 7; ++i) {
                player.hand.push_back(state_.draw_pile.back());
                state_.draw_pile.pop_back();
            }
        }
    
        // Flip first card
        state_.discard_pile.push_back(state_.draw_pile.back());
        state_.draw_pile.pop_back();
        state_.active_color = GetColor(state_.discard_pile.back());
    
        state_.status = MatchStatus::kPlaying;
    }
    
    void MatchInstance::Tick() {
        while (!state_.effect_queue.empty()) {
            auto& current_effect = state_.effect_queue.front();
            
            EffectResult result = current_effect->Resolve(&state_);
    
            if (result.status == EffectStatus::kNeedsInput) {
                state_.pending_player = result.target_player;
                state_.pending_input_type = result.input_type;
                return; // Yield thread
            }
    
            state_.effect_queue.pop_front();
        }
    
        state_.pending_player.clear();
        state_.pending_input_type.clear();
        state_.provided_input.clear();
    }
    
    bool MatchInstance::PlayCard(const std::string& username, uint16_t card_id) {
        if (IsWaitingForInput() || IsGameOver()) return false;
    
        Player* p = GetPlayer(username);
        if (!p) return false;
    
        if (state_.players[state_.current_player_index].username != username) return false;
    
        auto card_it = std::ranges::find(p->hand, card_id, GetId);
        if (card_it == p->hand.end()) return false;

        CompactCard played_card = *card_it;
        CardPlayedEvent event = { username, played_card, true, false };
    
        // 1. Validation Pipeline
        for (auto& rule : active_rules_) {
            rule->ValidatePlay(&state_, event);
            if (event.is_handled) break;
        }
        if (!event.is_valid_play) return false;
    
        // 2. Commit Card
        state_.discard_pile.push_back(played_card);
        state_.active_color = GetColor(played_card);
        p->hand.erase(card_it);
    
        // 3. Effects Pipeline
        for (auto& rule : active_rules_) {
            rule->OnCardPlayed(&state_, event);
            if (event.is_handled) break;
        }
    
        // 4. Game Over Check
        if (p->hand.empty()) {
            state_.status = MatchStatus::kFinished;
            state_.winner = username;
            return true;
        }
    
        AdvanceTurn();
        return true;
    }
    
    bool MatchInstance::DrawCard(const std::string& username) {
        if (IsWaitingForInput() || IsGameOver()) return false;
    
        Player* p = GetPlayer(username);
        if (!p) return false;
    
        if (state_.players[state_.current_player_index].username != username) return false;
    
        if (state_.draw_pile.empty()) {
            // Shuffle discard pile back into draw pile (leaving top card)
            if (state_.discard_pile.size() <= 1) return false; 
            
            CompactCard top = state_.discard_pile.back();
            state_.discard_pile.pop_back();
    
            state_.draw_pile = std::move(state_.discard_pile);
            state_.discard_pile.push_back(top);
    
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(state_.draw_pile.begin(), state_.draw_pile.end(), g);
        }
    
        p->hand.push_back(state_.draw_pile.back());
        state_.draw_pile.pop_back();
        p->has_called_uno = false; 
    
        AdvanceTurn();
        return true;
    }
    
    void MatchInstance::CallUno(const std::string& username) {
        Player* p = GetPlayer(username);
        if (p && p->hand.size() <= 2) {
            p->has_called_uno = true;
        }
    }
    
    void MatchInstance::ProvideInput(const std::string& username, const std::string& input) {
        if (username == state_.pending_player) {
            state_.provided_input = input;
        }
    }
    
    void MatchInstance::AdvanceTurn() {
        state_.current_player_index += state_.play_direction;
        int num_players = static_cast<int>(state_.players.size());
        
        if (state_.current_player_index >= num_players) {
            state_.current_player_index %= num_players;
        } else while (state_.current_player_index < 0) {
            state_.current_player_index += num_players;
        }
    }
    
    bool MatchInstance::TakeBotTurn() {
        if (IsGameOver()) return false;
    
        if (IsWaitingForInput()) {
            // Simple AI: Default to choosing RED when playing a Wild
            ProvideInput(state_.pending_player, "RED"); 
            Tick();
            return true;
        }
    
        Player& current_player = state_.players[state_.current_player_index];
    
        // Try to play the first legal card
        for (CompactCard card : current_player.hand) {
            CardPlayedEvent event = { current_player.username, card, true, false };
            
            for (auto& rule : active_rules_) {
                rule->ValidatePlay(&state_, event);
                if (event.is_handled) break;
            }
    
            if (event.is_valid_play) {
                PlayCard(current_player.username, GetId(card));
                
                // If playing the card caused an async wait (e.g., Wild), resolve it immediately
                if (IsWaitingForInput()) {
                    TakeBotTurn(); 
                }
                return true;
            }
        }
    
        // No valid cards, must draw
        DrawCard(current_player.username);
        return true;
    }
    
    std::string MatchInstance::GetCurrentPlayerUsername() const {
        if (state_.players.empty()) return "";
        return state_.players[state_.current_player_index].username;
    }
    
    Player* MatchInstance::GetPlayer(const std::string& username) {
        auto it = std::ranges::find(state_.players, username, &Player::username);
        return it != state_.players.end() ? &(*it) : nullptr;
    }
    
    void MatchInstance::GenerateDeck() {
        uint16_t unique_card_identifier = 0;
        Color standard_colors[] = {Color::kRed, Color::kBlue, Color::kGreen, Color::kYellow};
    
        for (Color current_color : standard_colors) {
            // Generate Zeros
            for (int i = 0; i < settings_.count_zeros; ++i) {
                state_.draw_pile.push_back(MakeCard(current_color, Value::k0, unique_card_identifier++));
            }
                
            // Generate Numbers 1-9
            for (int card_value = 1; card_value <= 9; ++card_value) {
                for (int i = 0; i < settings_.count_numbered; ++i) {
                    state_.draw_pile.push_back(MakeCard(current_color, static_cast<Value>(card_value), unique_card_identifier++));
                }
            }
            
            // Generate Skips
            for (int i = 0; i < settings_.count_skips; ++i) {
                state_.draw_pile.push_back(MakeCard(current_color, Value::kSkip, unique_card_identifier++));
            }
            
            // Generate Reverses (Blocks)
            for (int i = 0; i < settings_.count_reverses; ++i) {
                state_.draw_pile.push_back(MakeCard(current_color, Value::kReverse, unique_card_identifier++));
            }
            
            // Generate +2s
            for (int i = 0; i < settings_.count_draw_two; ++i) {
                state_.draw_pile.push_back(MakeCard(current_color, Value::kDraw2, unique_card_identifier++));
            }
        }

        // 2. Generate Wild Cards (Color-less)
        for (int i = 0; i < settings_.count_wild; ++i) {
            state_.draw_pile.push_back(MakeCard(Color::kWild, Value::kWild, unique_card_identifier++));
        }

        for (int i = 0; i < settings_.count_wild_draw_four; ++i) {
            state_.draw_pile.push_back(MakeCard(Color::kWild, Value::kWildDraw4, unique_card_identifier++));
        }
    
        std::random_device random_device;
        std::mt19937 random_generator(random_device());
        std::shuffle(state_.draw_pile.begin(), state_.draw_pile.end(), random_generator);
    }
    
    nlohmann::json MatchInstance::SerializePlayerState(const std::string& username) const {
        nlohmann::json root;
        root["match_status"] = static_cast<int>(state_.status);
        if (IsGameOver()) root["winner"] = state_.winner;
    
        root["active_color"] = static_cast<int>(state_.active_color);
        root["current_turn"] = state_.players[state_.current_player_index].username;
        root["play_direction"] = state_.play_direction;
    
        if (!state_.discard_pile.empty()) {
            CompactCard top = state_.discard_pile.back();
            root["top_card"] = {
                {"id", GetId(top)}, 
                {"color", static_cast<int>(GetColor(top))}, 
                {"value", static_cast<int>(GetValue(top))},
                {"discard_pile_size", state_.discard_pile.size()}
            };
        }
    
        nlohmann::json players_array = nlohmann::json::array();
        for (const auto& p : state_.players) {
            nlohmann::json p_json;
            p_json["username"] = p.username;
            p_json["card_count"] = p.hand.size();
            p_json["has_called_uno"] = p.has_called_uno;
    
            if (p.username == username) {
                nlohmann::json hand_json = nlohmann::json::array();
                for (CompactCard c : p.hand) {
                    hand_json.push_back({
                        {"id", GetId(c)}, 
                        {"color", static_cast<int>(GetColor(c))}, 
                        {"value", static_cast<int>(GetValue(c))}
                    });
                }
                p_json["hand"] = hand_json;
            }
            players_array.push_back(p_json);
        }
        root["players"] = players_array;
    
        return root;
    }  
}
