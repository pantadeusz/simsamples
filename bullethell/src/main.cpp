/**
 * This is the simple hello world for SDL2.
 *
 * You need C++14 to compile this.
 */

#include "lodepng.h"
#include "vectors.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

/*
  SDL_SetRenderDrawColor(renderer.get(), 55, 55, 20, 255);
  SDL_RenderClear(renderer.get());

  SDL_Surface *game_map_bmp = SDL_CreateRGBSurfaceWithFormatFrom(
      game_world.terrain->terrain.data(), game_world.terrain->w,
      game_world.terrain->h, 32, 4 * game_world.terrain->w,
      SDL_PIXELFORMAT_RGBA32);
  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer.get(), game_map_bmp);
  SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  SDL_RenderCopy(renderer.get(), tex, NULL, &dstrect);
  SDL_DestroyTexture(tex);
  SDL_FreeSurface(game_map_bmp);
  SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);
  SDL_RenderDrawLine(renderer.get(), (int)(camera_pos[0]), (int)(camera_pos[1]),
                     (int)(camera_pos[0]), (int)(camera_pos[1]) - 30);
  SDL_RenderPresent(renderer.get()); // draw frame to screen
  lodepng::decode(game_map.terrain, game_map.w, game_map.h, "data/plansza.png");
    auto kbdstate = SDL_GetKeyboardState(NULL);

*/


int main(int, char**)
{
    using namespace std;
    using namespace std::chrono;

    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    shared_ptr<SDL_Window> window_p(
        SDL_CreateWindow("Better Worms", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN),
        [](auto window) { SDL_DestroyWindow(window); });

    shared_ptr<SDL_Renderer> renderer_p(
        SDL_CreateRenderer(&*window_p, -1, SDL_RENDERER_ACCELERATED),
        [](auto renderer) {
            SDL_DestroyRenderer(renderer);
        }); // SDL_RENDERER_PRESENTVSYNC


    std::vector<double> p = {320, 0};
    std::vector<int> v = {3, 5};
    using namespace tp::operators;
    auto d = p + v;
    double l = length(d);
    SDL_Texture* tex = IMG_LoadTexture(&*renderer_p, "data/player.png");
    milliseconds dt(15);
    steady_clock::time_point current_time = steady_clock::now(); // remember current time
    for (bool game_active = true; game_active;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) { // check if there are some events
            if (event.type == SDL_QUIT)
                game_active = false;
        }
        auto kbdstate = SDL_GetKeyboardState(NULL);

        //    if (kbdstate[SDL_SCANCODE_LEFT])
        //    if (kbdstate[SDL_SCANCODE_RIGHT])
        //
        this_thread::sleep_until(current_time = current_time + dt);
    }
    SDL_Quit();
    return 0;
}


/**
 * see how this compiles
 * */
#ifdef DO_THE_GODBOLT_DEMO
#include <array>

template <typename T1, typename T2>
inline T1 operator+(const T1& a, const T2& b)
{
    T1 ret;
    for (unsigned i = 0; i < ((a.size() > b.size()) ? b.size() : a.size()); i++)
        ret[i] = a[i] + b[i];
    return ret;
}

template <typename T1, typename T2>
inline T1 operator-(const T1& a, const T2& b)
{
    T1 ret;
    for (unsigned i = 0; i < ((a.size() > b.size()) ? b.size() : a.size()); i++)
        ret[i] = a[i] - b[i];
    return ret;
}

template <typename T>
inline double length(const T& a)
{
    double ret = 0.0;
    for (unsigned i = 0; i < a.size(); i++)
        ret += a[i] * a[i];
    return (ret == 0) ? 0.0 : std::sqrt(ret);
}

int main(int argc)
{
    std::array<double, 2> p1 = {(double)argc, 1.0};
    std::array<double, 2> p2 = {2.0, (double)argc};
    auto r = p1 + p2;
    return r[0] + r[1];
}
#endif
