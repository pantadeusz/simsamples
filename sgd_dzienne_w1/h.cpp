#include "lodepng.h"
#include <SDL2/SDL.h>
#include <memory>
#include <stdexcept>
#include <vector>

class coord_t {
public:
  double x;
  double y;
  coord_t() : x(0), y(0) {}
  coord_t(const double x_, const double y_) {
    x = x_;
    y = y_;
  }
};

coord_t operator+(const coord_t &a, const coord_t &b) {
  return coord_t(a.x + b.x, a.y + b.y);
}
coord_t operator*(const coord_t &a, const double &b) {
  return coord_t(a.x * b, a.y * b);
}

class grass_t {
public:
  std::shared_ptr<SDL_Texture> tex;
  coord_t pos;
  coord_t v;

  grass_t(std::shared_ptr<SDL_Texture> t, double x, double y, double dx,
          double dy) {
    pos = {x, y};
    v = {dx, dy};
    tex = t;
  }

  void update(double dt) {
    pos = pos + v * dt;
    if (pos.x < -40) {
      pos.x = -40;
      v.x *= -1.0;
    }
    if (pos.x > 0) {
      pos.x = 0;
      v.x *= -1.0;
    }
  }
  void draw(SDL_Renderer *renderer) {
    SDL_Rect grass_position = {(int)pos.x, (int)pos.y, 360, 55};
    SDL_RenderCopy(renderer, tex.get(), NULL, &grass_position);
  }
};

class crosshair_t {
public:
  std::shared_ptr<SDL_Texture> tex;
  coord_t pos;

  crosshair_t() {}

  crosshair_t(std::shared_ptr<SDL_Texture> tex_, double x, double y) {
    pos = {x, y};
    tex = tex_;
  }
  void draw(SDL_Renderer *renderer) {
    SDL_Rect p = {(int)pos.x - 32, (int)pos.y - 32, 64, 64};
    SDL_RenderCopy(renderer, tex.get(), NULL, &p);
  }
};

class game_engine_t {

  static std::string assets_prefix;

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  std::vector<std::shared_ptr<SDL_Texture>> bg_layers;

  bool game_active = true;

  crosshair_t crosshair;

public:
  std::shared_ptr<SDL_Texture> load_img(std::string filename) {
    using namespace std;
    SDL_Surface *surface;
    std::shared_ptr<SDL_Texture> texture;
    vector<unsigned char> img_data;
    unsigned img_w, img_h;
    lodepng::decode(img_data, img_w, img_h, assets_prefix + filename);
    surface = SDL_CreateRGBSurfaceWithFormatFrom(
        img_data.data(), img_w, img_h, 32, 4 * img_w, SDL_PIXELFORMAT_RGBA32);

    texture = std::shared_ptr<SDL_Texture>(
        SDL_CreateTextureFromSurface(renderer, surface),
        [](SDL_Texture *t) { SDL_DestroyTexture(t); });
    if (!texture) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Couldn't create texture from surface: %s", SDL_GetError());
      throw std::invalid_argument(SDL_GetError());
    }
    SDL_FreeSurface(surface);
    return texture;
  }

  game_engine_t() {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                   SDL_GetError());
      throw std::invalid_argument(SDL_GetError());
    }

    if (SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_RESIZABLE, &window,
                                    &renderer)) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Couldn't create window and renderer: %s", SDL_GetError());
      throw std::invalid_argument(SDL_GetError());
    }
    SDL_RenderSetLogicalSize(renderer, 320, 240);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_ShowCursor(0);

    bg_layers = {load_img("background.png"), load_img("grass1.png"),
                 load_img("grass2.png")};
    crosshair = crosshair_t(load_img("crosshair.png"), 160, 100);
  }

  void game_loop() {
    std::vector<grass_t> grass;
    grass.push_back(grass_t(bg_layers[1], 0, 200, 10, 0));
    grass.push_back(grass_t(bg_layers[2], 0, 210, 13.123, 0));

    double dt = 0; // przyrost czasu w sekundach
    long int frame_number = 0;

    auto prev_tick = SDL_GetTicks();
    while (game_active) {
      // petla zdarzen
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
          game_active = false;
          break;
        case SDL_MOUSEMOTION:
          crosshair.pos.x = event.motion.x;
          crosshair.pos.y = event.motion.y;
          break;
        }
      }
      // fizyka

      if (dt > 0.0) {
        for (auto &g : grass)
          g.update(dt);
      }
      // grafika
      SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, bg_layers.at(0).get(), NULL, NULL);
      for (auto &g : grass)
        g.draw(renderer);

      crosshair.draw(renderer);
      SDL_RenderPresent(renderer);
      auto new_tick = SDL_GetTicks();
      dt = (new_tick - prev_tick) / 1000.0;
      frame_number += (new_tick - prev_tick);
      prev_tick = new_tick;
      if (dt > 0.00001)
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "t: %8d  Dt: %f\n",
                    frame_number, dt);
    }
  }
  ~game_engine_t() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
  }
};

std::string game_engine_t::assets_prefix = "assets/";

int main(int argc, char *argv[]) {
  using namespace std;
  game_engine_t engine;
  engine.game_loop();
  return 0;
}
