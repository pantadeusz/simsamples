/**
 * This is the simple hello world for SDL2.
 *
 * You need C++14 to compile this.
 */

#include "lodepng.h"
#include <SDL2/SDL.h>
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

// check for errors
#define errcheck(e)                                                            \
  {                                                                            \
    if (e) {                                                                   \
      cout << SDL_GetError() << endl;                                          \
      SDL_Quit();                                                              \
      return -1;                                                               \
    }                                                                          \
  }

class point_t : public std::array<double, 2> {
public:
  point_t() {
    operator[](0) = 0;
    operator[](1) = 1;
  }
  point_t(std::initializer_list<double> v) : std::array<double, 2>() {
    std::size_t i = 0;
    for (auto &e : v)
      this->operator[](i++) = e;
  };
  double &x() { return operator[](0); }
  double &y() { return operator[](1); }
  double x() const { return operator[](0); }
  double y() const { return operator[](1); }
  double len() const { return std::sqrt(x() * x() + y() * y()); }
};

point_t operator+(const point_t &a, const point_t &b) {
  return {a[0] + b[0], a[1] + b[1]};
}
point_t operator-(const point_t &a, const point_t &b) {
  return {a[0] - b[0], a[1] - b[1]};
}
point_t operator*(const point_t &a, const point_t &b) {
  return {a[0] * b[0], a[1] * b[1]};
}
point_t operator*(const point_t &a, const double b) {
  return {a[0] * b, a[1] * b};
}
point_t operator/(const point_t &a, const double b) {
  return {a[0] / b, a[1] / b};
}
///////////////////////////////////////////////////////////////////////////////////
struct player_t {
  point_t pos;
  point_t v;
  point_t a;
};

struct game_map_t {
  std::vector<unsigned char> terrain;
  unsigned w, h; // width, height;
  unsigned get(int x, int y) const {
    if (x < 0)
      throw std::invalid_argument("x too small");
    if (x >= w)
      throw std::invalid_argument("x too big");
    if (y < 0)
      throw std::invalid_argument("y too small");
    if (y >= h)
      throw std::invalid_argument("y too big");
    return *(uint32_t *)&terrain.data()[(y * w + x) * 4];
  }
  unsigned &get(int x, int y) {
    if (x < 0)
      throw std::invalid_argument("x too small");
    if (x >= w)
      throw std::invalid_argument("x too big");
    if (y < 0)
      throw std::invalid_argument("y too small");
    if (y >= h)
      throw std::invalid_argument("y too big");
    return *(uint32_t *)&terrain.data()[(y * w + x) * 4];
  }
  unsigned get(point_t p) const {
    point_t pp = p_to_map(p);
    return get((int)pp[0], (int)pp[1]);
  }
  point_t p_to_map(point_t p) const { return {p[0], h - p[1] - 1}; }
  point_t map_to_p(point_t p) const { return {p[0], 1.0 - p[1] + h}; }

  void draw_p_hole(point_t pos, double r) {
    auto p = p_to_map(pos);
    std::cout << pos[0] << " " << pos[1] << std::endl;
    draw_hole(p, r);
  }
  void draw_hole(point_t p, double r) {
    std::cout << "                       " << p[0] << " " << p[1] << " " << r
              << std::endl;
    unsigned *data = (unsigned *)terrain.data();
    for (double x = 0; x < r; x++) {
      for (double y = 0; y < r; y++) {
        if ((x * x + y * y) < r) {
          int xx, yy;
          int p0 = p[0], p1 = p[1];
          xx = p0 + x;
          yy = p1 + y;
          if ((xx >= 0) && (xx < w) && (yy > 0) && (yy < h))
            data[(yy * w + xx)] = 0;
          xx = p0 - x;
          yy = p1 + y;
          if ((xx >= 0) && (xx < w) && (yy > 0) && (yy < h))
            data[(yy * w + xx)] = 0;
          xx = p0 + x;
          yy = p1 - y;
          if ((xx >= 0) && (xx < w) && (yy > 0) && (yy < h))
            data[(yy * w + xx)] = 0;
          xx = p0 - x;
          yy = p1 - y;
          if ((xx >= 0) && (xx < w) && (yy > 0) && (yy < h))
            data[(yy * w + xx)] = 0;
        }
      }
    }
  }
};

struct game_world_t {
  player_t player;
  std::shared_ptr<game_map_t> terrain;
};

struct graphic_data_t {
  int width = 1024;
  int height = 768;
};

game_world_t new_game_state(const game_map_t &graphic_data) {
  game_world_t game;
  game.terrain = std::make_shared<game_map_t>(graphic_data);
  game.player.pos[0] = game.terrain->w / 2;
  game.player.pos[1] = 0;

  game.player.v[0] = 0;
  game.player.v[1] = 0;

  // for (int y = 3; y < game.terrain->h-3; y++) {
  //  for (int x = 308; x < 309; x++) {
  //  game.player.pos = {(double)x, (double)y};
  //  std::cout << " " << std::hex << std::setw(8) <<
  //  (int)((game.terrain->get(game.player.pos)));
  //  }
  //  std::cout << std::endl;
  //}

  for (int y = 3; y < game.terrain->h - 3; y++) {
    game.player.pos = {(double)(308), (double)y};

    std::cout << " " << std::hex << std::setw(8)
              << (int)((game.terrain->get(game.player.pos))) << std::endl;

    if ((game.terrain->get(game.player.pos) & 0x0ff000000) < 0x00f000000) {
      std::cout << "FOUND: " << game.player.pos[0] << " " << game.player.pos[1]
                << std::endl;
      game.player.pos[1] += 3;
      break;
    }
  }
  std::cout << "Start at: " << game.player.pos[0] << " " << game.player.pos[1]
            << std::endl;

  return game;
}

game_world_t do_simulation_step(const game_world_t &old_world,
                                std::chrono::milliseconds dt_) {
  auto new_world = old_world;
  double dt = (dt_.count() / 1000.0);
  double coeff = 0.2;
  new_world.player.pos = old_world.player.pos + old_world.player.v * dt +
                         (old_world.player.a * dt * dt) / 2;
  new_world.player.v = old_world.player.v + old_world.player.a * dt;

  new_world.player.v = new_world.player.v - point_t{0.0, 500.0 * dt};
  new_world.player.v = new_world.player.v - new_world.player.v * coeff * dt;

  if ((old_world.terrain->get(new_world.player.pos) & 0x0ff000000) >=
      0x0f000000) {
    new_world.player.pos = old_world.player.pos;
    new_world.player.a = {0, 0};
    new_world.player.v = {0, -(0.9) * new_world.player.v[1]};
  }

  // std::cout << new_world.player.pos[0] << " " << new_world.player.pos[1] <<
  // std::hex << std::setw(8) << old_world.terrain->get(new_world.player.pos)
  //          << std::endl;
  return new_world;
}

void render_everything(std::shared_ptr<SDL_Renderer> renderer,
                       const game_world_t &game_world,
                       const graphic_data_t &graphic_data) {
  SDL_SetRenderDrawColor(renderer.get(), 55, 55, 20, 255);
  SDL_RenderClear(renderer.get());

  // BITMAPA GRY!!!
  SDL_Surface *game_map_bmp = SDL_CreateRGBSurfaceWithFormatFrom(
      game_world.terrain->terrain.data(), game_world.terrain->w,
      game_world.terrain->h, 32, 4 * game_world.terrain->w,
      SDL_PIXELFORMAT_RGBA32);

  //  std::copy(game_world.terrain->terrain.begin(),
  //  game_world.terrain->terrain.end(),
  //            (unsigned char *)game_map_bmp->pixels);
  if (game_map_bmp == nullptr)
    throw "nie zaladowano surface";

  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer.get(), game_map_bmp);

  if (tex == nullptr)
    throw "zle";
  SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  auto mpos = game_world.terrain->p_to_map(game_world.player.pos);
  point_t camera_pos = {graphic_data.width / 2.0, graphic_data.height / 2.0};
  // std::cout << "P:" << mpos[0] << " " << mpos[1] << std::endl;
  SDL_Rect dstrect = {(int)(-mpos[0] + camera_pos[0]),
                      (int)(-mpos[1] + camera_pos[1]),
                      (int)game_world.terrain->w, (int)game_world.terrain->h};
  //  SDL_Rect dstrect = {0,//(int)(-mpos[0]+camera_pos[0]),
  //                      0,//(int)(-mpos[1]+camera_pos[1]),
  //                      (int)game_world.terrain->w,
  //                      (int)game_world.terrain->h};
  SDL_RenderCopy(renderer.get(), tex, NULL, &dstrect);
  SDL_DestroyTexture(tex);
  SDL_FreeSurface(game_map_bmp);

  /// rysuj graczaa i inne elementy

  SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);

  SDL_RenderDrawLine(renderer.get(), (int)(camera_pos[0]), (int)(camera_pos[1]),
                     (int)(camera_pos[0]), (int)(camera_pos[1]) - 30);
  //  SDL_RenderDrawLine(renderer.get(), (int)(mpos[0]),
  //   (int)(mpos[1]),
  //                     (int)(mpos[0]), (int)(mpos[1]) - 30);

  SDL_RenderPresent(renderer.get()); // draw frame to screen
}

int main(int, char **) {
  using namespace std;
  using namespace std::chrono;

  graphic_data_t graphic_data;

  game_world_t game_world;
  errcheck(SDL_Init(SDL_INIT_EVERYTHING) != 0);
  shared_ptr<SDL_Window> window(
      SDL_CreateWindow("Better Worms", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, graphic_data.width,
                       graphic_data.height, SDL_WINDOW_SHOWN),
      [](SDL_Window *window) { SDL_DestroyWindow(window); });
  errcheck(window == nullptr);

  shared_ptr<SDL_Renderer> renderer(
      SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED),
      [](SDL_Renderer *renderer) {
        SDL_DestroyRenderer(renderer);
      }); // SDL_RENDERER_PRESENTVSYNC
  errcheck(renderer == nullptr);
  game_map_t game_map;
  lodepng::decode(game_map.terrain, game_map.w, game_map.h, "data/plansza.png");

  game_world = new_game_state(game_map);

  // auto dt = 15ms;
  milliseconds dt(15);

  steady_clock::time_point current_time =
      steady_clock::now(); // remember current time
  for (bool game_active = true; game_active;) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) { // check if there are some events
      if (event.type == SDL_QUIT)
        game_active = false;
    }

    auto kbdstate = SDL_GetKeyboardState(NULL);
    double accv = 1000.0;
    game_world.player.a = {0.0, 0.0};
    if (kbdstate[SDL_SCANCODE_LEFT])
      game_world.player.a[0] = -accv;
    if (kbdstate[SDL_SCANCODE_RIGHT])
      game_world.player.a[0] = accv;
    if (kbdstate[SDL_SCANCODE_UP])
      game_world.player.a[1] = accv;
    if (kbdstate[SDL_SCANCODE_DOWN])
      game_world.player.a[1] = -accv;
    if (kbdstate[SDL_SCANCODE_SPACE]) {
      game_world.terrain->draw_p_hole(game_world.player.pos, 60);
    }

    // draw_hole(point_t pos, double r, std::vector<unsigned char> &game_map,
    // int width, int height )

    game_world = do_simulation_step(game_world, dt);
    render_everything(renderer, game_world, graphic_data);

    this_thread::sleep_until(current_time = current_time + dt);
  }
  SDL_Quit();
  return 0;
}
