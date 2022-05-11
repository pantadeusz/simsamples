/**
 * This is the simple hello world for SDL2.
 * 
 * You need C++14 to compile this.
 */

#include <SDL2/SDL.h>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

// check for errors
#define errcheck(e)                                                            \
  {                                                                            \
    if (e) {                                                                   \
      cout << SDL_GetError() << endl;                                          \
      SDL_Quit();                                                              \
      return -1;                                                               \
    }                                                                          \
  }

int main(int , char **) {
  using namespace std;
  using namespace std::chrono;
  const int width = 640;
  const int height = 480;

  errcheck(SDL_Init(SDL_INIT_VIDEO) != 0);

  SDL_Window *window = SDL_CreateWindow(
      "My Next Superawesome Game", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
  errcheck(window == nullptr);

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED); // SDL_RENDERER_PRESENTVSYNC
  errcheck(renderer == nullptr);

  //auto dt = 15ms;
  milliseconds dt(15);

  steady_clock::time_point current_time = steady_clock::now(); // remember current time
  for (bool game_active = true; game_active;) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) { // check if there are some events
      if (event.type == SDL_QUIT)
        game_active = false;
    }

    SDL_RenderPresent(renderer); // draw frame to screen

    this_thread::sleep_until(current_time = current_time + dt);
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
