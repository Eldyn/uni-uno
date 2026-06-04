#include <game/match_instance.hpp>
#include <game/rules/standard.hpp>
#include <game/effects/standard.hpp>
#include <common/game/effect.hpp>
#include <controllers/lobby_controller.hpp>
#include <algorithm>
#include <random>

namespace game {
    void ReshuffleDiscardIntoDraw(GameState* game_state) {
        if (game_state->discard_pile.size() <= 1) return; 
        
        CompactCard top_card = game_state->discard_pile.back();
        game_state->discard_pile.pop_back();
    
        game_state->draw_pile = std::move(game_state->discard_pile);
        game_state->discard_pile.push_back(top_card);
    
        std::random_device random_device;
        std::mt19937 random_generator(random_device());
        std::shuffle(game_state->draw_pile.begin(), game_state->draw_pile.end(), random_generator);
    }

    MatchInstance::MatchInstance(const std::vector<std::string>& usernames, const LobbySettings& settings) : settings_(settings) {
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
        
        for (auto& player : state_.players) {
            for (int index = 0; index < settings_.starting_cards; ++index) {
                player.hand.push_back(state_.draw_pile.back());
                state_.draw_pile.pop_back();
            }
        }
    
        state_.discard_pile.push_back(state_.draw_pile.back());
        state_.draw_pile.pop_back();
        state_.active_color = GetColor(state_.discard_pile.back());
    
        state_.status = MatchStatus::kPlaying;
    }
    
    void MatchInstance::Tick() {
        while (!state_.effect_queue.empty()) {
            // 1. EXTRACT the effect completely so push_front doesn't mess up the pop order!
            auto current_effect = std::move(state_.effect_queue.front());
            state_.effect_queue.pop_front();
            
            // 2. Resolve the extracted effect
            EffectResult result = current_effect->Resolve(&state_, this); 
    
            if (result.status == EffectStatus::kNeedsInput) {
                state_.pending_player = result.target_player;
                state_.pending_input_type = result.input_type;
                state_.pending_input_context = result.input_context;
                
                // 3. Put it back at the front of the queue so it resumes when the player clicks a button
                state_.effect_queue.push_front(std::move(current_effect));
                return; 
            }
        }
    }
    
    bool MatchInstance::PlayCard(const std::string& username, uint16_t card_id) {
        if (IsWaitingForInput() || IsGameOver()) return false;
    
        Player* current_player = GetPlayer(username);
        if (!current_player) return false;
    
        if (state_.players[state_.current_player_index].username != username) return false;
    
        auto card_iterator = std::ranges::find(current_player->hand, card_id, GetId);
        if (card_iterator == current_player->hand.end()) return false;
    
        CompactCard played_card = *card_iterator;
        CardPlayedEvent play_event = { username, played_card, true, false };
    
        for (auto& rule : active_rules_) {
            rule->ValidatePlay(&state_, play_event);
            if (play_event.is_handled) break;
        }
        
        if (!play_event.is_valid_play) return false;
    
        state_.discard_pile.push_back(played_card);
        state_.active_color = GetColor(played_card);
        current_player->hand.erase(card_iterator);
    
        for (auto& rule : active_rules_) {
            rule->OnCardPlayed(&state_, play_event);
            if (play_event.is_handled) break;
        }
    
        state_.effect_queue.push_back(std::make_unique<AdvanceTurnEffect>());
    
        // TODO: propagate to players the game end
        if (current_player->hand.empty()) {
            state_.status = MatchStatus::kFinished;
            state_.winner = username;
            return true;
        }
    
        return true;
    }
    
    bool MatchInstance::DrawCard(const std::string& username) {
        if (IsWaitingForInput() || IsGameOver()) return false;
    
        Player* current_player = GetPlayer(username);
        if (!current_player) return false;
    
        if (state_.players[state_.current_player_index].username != username) return false;
    
        if (state_.draw_pile.empty()) {
            ReshuffleDiscardIntoDraw(&state_);
            if (state_.draw_pile.empty()) return false; 
        }
    
        CompactCard drawn_card = state_.draw_pile.back();
        current_player->hand.push_back(drawn_card);
        state_.draw_pile.pop_back();
        
        current_player->has_called_uno = false; 
    
        // Check if the card is playable
        CardPlayedEvent dummy_event = { username, drawn_card, true, false };
        for (auto& rule : active_rules_) {
            rule->ValidatePlay(&state_, dummy_event);
            if (dummy_event.is_handled) break;
        }
    
        if (dummy_event.is_valid_play) {
            // Push the effect that prompts the user and awaits their decision!
            state_.effect_queue.push_front(std::make_unique<DecideDrawnCardEffect>(username, GetId(drawn_card)));
        } else {
            state_.effect_queue.push_back(std::make_unique<AdvanceTurnEffect>());
        }
    
        return true;
    }
    
    void MatchInstance::ProvideInput(const std::string& username, const std::string& input) {
        if (username == state_.pending_player) {
            state_.provided_input = input;
            
            // Instantly unlock the engine so Effects can execute methods safely
            state_.pending_player.clear();
            state_.pending_input_type.clear();
            state_.pending_input_context.clear();
        }
    }

    void MatchInstance::CallUno(const std::string& username) {
        Player* p = GetPlayer(username);
        if (p && p->hand.size() <= 2) {
            p->has_called_uno = true;
        }
    }
    
    bool MatchInstance::TakeBotTurn() {
        if (IsGameOver()) return false; 

        if (IsWaitingForInput()) {
            if (state_.pending_input_type == "play_drawn_card") {
                ProvideInput(state_.pending_player, "PLAY");
            } else {
                ProvideInput(state_.pending_player, "RED");
            }
            
            Tick();
            
            if (IsWaitingForInput()) TakeBotTurn(); 
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
