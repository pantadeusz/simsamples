/**
 * This is the simple game for SDL2 and SDL2_Image.
 *
 * You need C++14 to compile this.
 * 
 * Tadeusz Puźniakowski 2021
 * 
 * Unlicensed
 */

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

void draw_o(SDL_Renderer* r, std::array<double, 2> p, SDL_Texture* tex, double w, double h, double a)
{
    SDL_Rect dst_rect = {(int)(p[0] - w / 2), (int)(p[1] - h / 2), (int)w, (int)h};
    SDL_RenderCopyEx(&*r, &*tex, NULL, &dst_rect, a, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
}

int main(int, char**)
{
    using namespace std;
    using namespace std::chrono;

    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    shared_ptr<SDL_Window> window_p(
        SDL_CreateWindow("Better Worms", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, 640, 360, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
        [](auto* window) { SDL_DestroyWindow(window); });

    shared_ptr<SDL_Renderer> renderer_p(
        SDL_CreateRenderer(&*window_p, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        [](auto* renderer) {
            SDL_DestroyRenderer(renderer);
        });

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_RenderSetLogicalSize(&*renderer_p, 320, 180);

    shared_ptr<SDL_Texture> tex_p(IMG_LoadTexture(&*renderer_p, "data/player.png"),
        [](auto* tex) { SDL_DestroyTexture(tex); });

    milliseconds dt(15);
    steady_clock::time_point current_time = steady_clock::now(); // remember current time
    for (bool game_active = true; game_active;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) { // check if there are some events
            if (event.type == SDL_QUIT)
                game_active = false;
        }
        //auto kbdstate = SDL_GetKeyboardState(NULL);
        //    if (kbdstate[SDL_SCANCODE_LEFT])
        //    if (kbdstate[SDL_SCANCODE_RIGHT])

        SDL_SetRenderDrawColor(&*renderer_p, 0, 100, 20, 255);
        SDL_RenderClear(&*renderer_p);
        SDL_SetRenderDrawColor(&*renderer_p, 255, 100, 200, 255);
        SDL_RenderCopy(&*renderer_p, &*tex_p, NULL, NULL);
        draw_o(&*renderer_p,{10,20},&*tex_p,16,16,0);
        draw_o(&*renderer_p,{50,20},&*tex_p,16,16,30);
        SDL_RenderPresent(&*renderer_p);
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
