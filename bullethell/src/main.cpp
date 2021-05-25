/**
 * This is the simple game for SDL2 and SDL2_Image.
 *
 * You need C++14 to compile this.
 * 
 * Tadeusz Puźniakowski 2021
 * 
 * Unlicensed
 */


/**
 * TODO:
 * 
 * Code reorg
 * Multiplayer
 * Keyboard mapping
 * 
 * 
 * */

/**
 * The game world is 10x10 tiles. Every coordinate is multiplied by 10 in order to drive it.
 * 
 * */

#include "bmpfont.hpp"
#include "vectors.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "main.hpp"

std::ostream& operator<<(std::ostream& o, const std::array<double, 2>& a)
{
    o << "[" << a[0] << "," << a[1] << "]";
    return o;
}

void draw_o(std::shared_ptr<SDL_Renderer> r, std::array<double, 2> p, std::shared_ptr<SDL_Texture> tex, double w, double h, double a)
{
    SDL_Rect dst_rect = {(int)(p[0] - w / 2), (int)(p[1] - h / 2), (int)w, (int)h};
    SDL_RenderCopyEx(r.get(), tex.get(), NULL, &dst_rect, a, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
}


int process_input(game_c& game)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) { // check if there are some events
        if (event.type == SDL_QUIT)
            return false;
    }
    auto kbdstate = SDL_GetKeyboardState(NULL);
    for (unsigned i = 0; i < game.players.size(); i++) {
        game.players.at(i).intentions.clear();
        for (auto [k, v] : game.keyboard_map.at(i)) {
            if (kbdstate[v]) game.players.at(i).intentions[k] = 1;
        }
    }
    return true;
}

void process_events(game_c& game)
{
    double dt_f = game.dt.count() / 1000.0;
    /// apply safe place and hit points
    for (unsigned i = 0; i < game.players.size(); i++) {
        auto& p = game.players[i];
        if ((p.is_safe_place())) {
            p.health += dt_f * 10.0;
            if (p.health > 100.0) p.health = 100;
        } else {
            p.points += dt_f * 10.0;
        }
    }

    for (unsigned i = 0; i < game.emitters.size(); i++) {
        auto& e = game.emitters[i];
        e.emit_to_emit -= dt_f;

        if (e.emit_to_emit <= 0.0) {
            e.emit_to_emit = e.emit_delay;
            game.bullets.push_back(bullet_c({e.position, {-10.0, -0.0}, {0.0, 1.0},0.0,"bullet[0]"}));
        }
    }
}

void process_physics(game_c& game)
{
    using namespace tp::operators;
    double dt_f = game.dt.count() / 1000.0;

    /// fizyka
    auto old_players = game.players;
    // update moves
    for (auto& player : game.players) {
        player.apply_intent();
        player.update(dt_f);
    }
    // update bullets
    std::vector<bullet_c> bullets_new;

    for (auto& bullet : game.bullets) {
        bullet.update(dt_f);
        if ((bullet.position[0] > -10.0) && (bullet.position[0] < 74) && (bullet.position[0] > -1000.0) && (bullet.position[1] < 74)) {
            bullets_new.push_back(bullet);
        }
    }
    std::swap(bullets_new, game.bullets);
    // check colision between players
    // they can interact with each other
    for (unsigned i = 0; i < game.players.size(); i++) {
        for (unsigned j = i + 1; j < game.players.size(); j++) {
            if (length(game.players[i].position - game.players[j].position) < 1.0) {
                game.players[i].position = old_players[i].position;
                game.players[j].position = old_players[j].position;
                auto vec = game.players[i].position - game.players[j].position;
                vec = vec * (1.0 / length(vec));
                game.players[i].velocity = vec; //old_players[i].position;
                game.players[j].velocity = vec * -1.0;
            }
        }
    }
    // TODO

    // check collisions with ground - always active
    for (unsigned i = 0; i < game.players.size(); i++) {
        if (game.players[i].position[1] < 32) {
            game.players[i].friction = 0.2;
        } else {
            game.players[i].velocity = {(game.players[i].velocity[0] * game.players[i].velocity[0] > 2.2) ? game.players[i].velocity[0] : 0.0, 0};
            game.players[i].position[1] = 32;
            game.players[i].friction = 0.3;
        }
    }
}

void process_sound(game_c&)
{
    // todo for students :)
}

/// player size i 10 x 10
void draw_scene(game_c& game)
{
    using namespace tp::operators;

    SDL_SetRenderDrawColor(game.renderer_p.get(), 0, 100, 20, 255);
    SDL_RenderClear(game.renderer_p.get());
    SDL_SetRenderDrawColor(game.renderer_p.get(), 255, 100, 200, 255);

    // DRAW ALL PLAYERS
    for (unsigned i = 0; i < game.players.size(); i++) {
        auto& player = game.players[i];
        draw_o(game.renderer_p, player.position * 10.0, game.textures.at("player[" + std::to_string(i) + "]"), 16, 16, player.position[0] * 36 + player.position[1] * 5);
        if (player.is_safe_place())
            draw_o(game.renderer_p, player.position * 10.0, game.textures.at("player[" + std::to_string(i) + "]"), 16 + 4, 16 + 4, player.position[0] * 36 + player.position[1] * 5);

        tp::draw_text(game.renderer_p, 10 + i * 130, 340, game.textures["font_10_red"], std::to_string((int)player.health));
        tp::draw_text(game.renderer_p, 10 + i * 130 + 40, 340, game.textures["font_10_blue"], std::to_string((int)player.points));
    }
    // DRAW ALL EMITTERS
    for (unsigned i = 0; i < game.emitters.size(); i++) {
        auto& emitter = game.emitters[i];
        draw_o(game.renderer_p, emitter.position * 10.0, game.textures.at("emitter[" + std::to_string(i) + "]"), 16, 16, 0.0);
    }
    // DRAW ALL BULLETS
    for (unsigned i = 0; i < game.bullets.size(); i++) {
        auto& bullet = game.bullets[i];
        draw_o(game.renderer_p, bullet.position * 10.0, game.textures.at(bullet.type), 10, 10, 33.0);
    }

    SDL_RenderPresent(game.renderer_p.get());
}


int main(int, char**)
{
    using namespace std;
    using namespace std::chrono;

    auto game = initialize_all();
    steady_clock::time_point current_time = steady_clock::now(); // remember current time
    for (bool game_active = true; game_active;) {
        game_active = process_input(game);
        process_events(game);
        process_physics(game);
        draw_scene(game);

        this_thread::sleep_until(current_time = current_time + game.dt);
    }
    SDL_Quit();
    return 0;
}
