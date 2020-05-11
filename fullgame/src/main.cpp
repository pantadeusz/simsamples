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
  double len() const {return std::sqrt(x()*x() + y()*y());}
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

struct player_t {
  point_t pos;
  point_t v;
  point_t a;
};

struct game_world_t {
  player_t player;
};

struct graphic_data_t {
  int width = 640;
  int height = 480;
  std::vector<unsigned char> game_map;
  unsigned game_map_width, game_map_height;
  unsigned get_color(int x, int y) const {
    return *(unsigned *)&game_map[y*game_map_width+x];
  }
};


point_t game_coord_to_map_coord(point_t p, int width, int height) {
  return {p[0], height - p[1] - 1};
}
game_world_t new_game_state(const graphic_data_t &graphic_data ) {
  game_world_t game;
  game.player.pos[0] = graphic_data.game_map_width/2;
  game.player.pos[1] = 0;

  game.player.v[0] = 0;
  game.player.v[1] = 0;

  
  for (int y = 0; y < graphic_data.game_map_width/2-1; y++) {
    
    if ((graphic_data.get_color(game.player.pos[0],y+graphic_data.game_map_width/2) & 0x0ff) == 0) {
      game.player.pos[1] = -y+graphic_data.game_map_width/2;
      std::cout << game.player.pos[0] << " " << game.player.pos[1] << std::endl;
      break;
    }
  } 

  return game;
}

game_world_t do_simulation_step(const game_world_t &old_world,const graphic_data_t &graphic_data, 
                                std::chrono::milliseconds dt_) {
  auto new_world = old_world;
  //new_world.player.pos =
  //    new_world.player.pos + (new_world.player.v * (dt.count() / 1000.0));
  double dt = (dt_.count() / 1000.0);
  double coeff = 0.2;
  new_world.player.pos = old_world.player.pos + old_world.player.v*dt + (old_world.player.a*dt*dt)/2;
  new_world.player.v = old_world.player.v + old_world.player.a*dt;
  
  new_world.player.v = new_world.player.v - point_t{0.0,500.0*dt};
  new_world.player.v = new_world.player.v - new_world.player.v*coeff*dt;

  auto map_pos = game_coord_to_map_coord(
      new_world.player.pos,
      graphic_data.game_map_width,
      graphic_data.game_map_height);
  if ((graphic_data.get_color(map_pos[0],map_pos[1]) & 0x0ff) >= 10) {
    new_world.player.pos = old_world.player.pos;
    new_world.player.a = {0,0};
    new_world.player.v = {0,-(0.9)*new_world.player.v[1]};
  }

  std::cout << new_world.player.pos[0] << " " << new_world.player.pos[1] << std::endl;
  return new_world;
}




void draw_hole(point_t pos, double r, std::vector<unsigned char> &game_map, int width, int height ) {
  // [(y*width+x)*4]
  auto p = game_coord_to_map_coord(pos,width, height);
  std::cout << p[0] << " " << p[1] << std::endl;
  for (double x = 0; x < r; x++) {
  for (double y = 0; y < r; y++) {
    if ((x*x+y*y) < r) {
      int xx = p[0]+x;
      int yy = p[1]+y;
      if ((xx >= 0) && (xx < width) && (yy > 0) && (yy < height))
        ((unsigned *)game_map.data())[(yy*width+xx)] = 0;
      xx = p[0]-x;
      yy = p[1]+y;
      if ((xx >= 0) && (xx < width) && (yy > 0) && (yy < height))
        ((unsigned *)game_map.data())[(yy*width+xx)] = 0;
      xx = p[0]+x;
      yy = p[1]-y;
      if ((xx >= 0) && (xx < width) && (yy > 0) && (yy < height))
        ((unsigned *)game_map.data())[(yy*width+xx)] = 0;
      xx = p[0]-x;
      yy = p[1]-y;
      if ((xx >= 0) && (xx < width) && (yy > 0) && (yy < height))
        ((unsigned *)game_map.data())[(yy*width+xx)] = 0;
    }
  }
  }
}


void render_everything(std::shared_ptr<SDL_Renderer> renderer,
                       const game_world_t &game_world,
                       const graphic_data_t &graphic_data) {
  SDL_SetRenderDrawColor(renderer.get(), 55, 55, 20, 255);
  SDL_RenderClear(renderer.get());

  // BITMAPA GRY!!!
  SDL_Surface *game_map_bmp = SDL_CreateRGBSurfaceWithFormat(
      0, graphic_data.game_map_width, graphic_data.game_map_height, 32,
      SDL_PIXELFORMAT_RGBA32);
  std::copy(graphic_data.game_map.begin(), graphic_data.game_map.end(),
            (unsigned char *)game_map_bmp->pixels);
  if (game_map_bmp == nullptr)
    throw "nie zaladowano surface";

  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer.get(), game_map_bmp);

  if (tex == nullptr)
    throw "zle";
  SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  SDL_Rect dstrect = {
    (int)(-game_world.player.pos[0]+graphic_data.width/2), 
    (int)(game_world.player.pos[1]+graphic_data.height/2-graphic_data.game_map_height),
    (int)graphic_data.game_map_width,
                      (int)graphic_data.game_map_height};
  SDL_RenderCopy(renderer.get(), tex, NULL, &dstrect);
  SDL_DestroyTexture(tex);
  SDL_FreeSurface(game_map_bmp);

  /// rysuj graczaa i inne elementy

  SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);

  SDL_RenderDrawLine(renderer.get(), graphic_data.width/2,
                     graphic_data.height/2,
                     graphic_data.width/2,
                     graphic_data.height/2 - 30);

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

  lodepng::decode(graphic_data.game_map, graphic_data.game_map_width, graphic_data.game_map_height,
                  "data/plansza.png");

  game_world = new_game_state(graphic_data);

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
    game_world.player.a = {0.0,0.0};
    if (kbdstate[SDL_SCANCODE_LEFT])
      game_world.player.a[0] = -accv;
    if (kbdstate[SDL_SCANCODE_RIGHT])
      game_world.player.a[0] = accv;
    if (kbdstate[SDL_SCANCODE_UP])
      game_world.player.a[1] = accv;
    if (kbdstate[SDL_SCANCODE_DOWN])
      game_world.player.a[1] = -accv;
    if (kbdstate[SDL_SCANCODE_SPACE]) {
      draw_hole(game_world.player.pos, 30, graphic_data.game_map, graphic_data.game_map_width, graphic_data.game_map_height );
    }

    // draw_hole(point_t pos, double r, std::vector<unsigned char> &game_map, int width, int height )

    game_world = do_simulation_step(game_world,graphic_data, dt);
    render_everything(renderer, game_world, graphic_data);

    this_thread::sleep_until(current_time = current_time + dt);
  }
  SDL_Quit();
  return 0;
}
