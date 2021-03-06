#include "lodepng.h"
#include "net/client.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <random>
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
  double length2() const { return (x * x) + (y * y); }
  double length() const { return sqrt(length2()); }
};

coord_t operator+(const coord_t &a, const coord_t &b) {
  return coord_t(a.x + b.x, a.y + b.y);
}
coord_t operator-(const coord_t &a, const coord_t &b) {
  return coord_t(a.x - b.x, a.y - b.y);
}
coord_t operator*(const coord_t &a, const double &b) {
  return coord_t(a.x * b, a.y * b);
}
coord_t operator*(const coord_t &a, const coord_t &b) {
  return coord_t(a.x * b.x, a.y * b.y);
}

std::shared_ptr<SDL_Texture> load_img(SDL_Renderer *renderer,
                                      std::string filename, int flag = 0,
                                      Uint32 key = 0) {
  using namespace std;
  static std::string assets_prefix = "assets/";
  SDL_Surface *surface;
  std::shared_ptr<SDL_Texture> texture;
  vector<unsigned char> img_data;
  unsigned img_w, img_h;
  lodepng::decode(img_data, img_w, img_h, assets_prefix + filename);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "color at 0,0 %x",
              *(Uint32 *)img_data.data());
  surface = SDL_CreateRGBSurfaceWithFormatFrom(
      img_data.data(), img_w, img_h, 32, 4 * img_w, SDL_PIXELFORMAT_RGBA32);

  if (flag)
    SDL_SetColorKey(surface, flag, key);

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

class spritesheet_t {
public:
  SDL_Renderer *renderer;
  std::shared_ptr<SDL_Texture> tex;
  std::vector<SDL_Rect> frames;
  std::vector<int> delays;

  int current_anim_frame;
  int current_anim_time;

  spritesheet_t(SDL_Renderer *renderer_, std::string fname,
                const SDL_Rect first_frame, int frames_count, int default_delay,
                int use_keying = 0, Uint32 key = 0) {
    current_anim_frame = 0;
    current_anim_time = 0;
    renderer = renderer_;
    tex = load_img(renderer, fname, use_keying, key);
    SDL_Rect curr_frame = first_frame;
    for (int i = 0; i < frames_count; i++) {
      frames.push_back(curr_frame);
      delays.push_back(default_delay);
      curr_frame.x += curr_frame.w;
    }
  }
  void draw(coord_t pos, const coord_t v = {0, 0}) {
    SDL_Rect dest = {(int)pos.x, (int)pos.y, frames.at(current_anim_frame).w,
                     frames.at(current_anim_frame).h};
    if (v.x == 0) {
      SDL_RenderCopy(renderer, tex.get(), &frames.at(current_anim_frame),
                     &dest);
    } else {
      SDL_RenderCopyEx(renderer, tex.get(), &frames.at(current_anim_frame),
                       &dest, 0.0, NULL,
                       (v.x < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
  }

  void update(int df) {
    current_anim_time += df;
    if (current_anim_time > delays.at(current_anim_frame)) {
      current_anim_time -= delays.at(current_anim_frame);
      current_anim_frame = (current_anim_frame + 1) % delays.size();
    }
  }
};

class duck_t {
public:
  std::shared_ptr<spritesheet_t> anim;

  coord_t pos;
  coord_t v;
  coord_t a;

  void respawn(coord_t pos_, double angle, double v_) {
    a.x = 0;
    a.y = 100.0;
    v = {cos(angle) * v_, sin(angle) * v_};
    pos = pos_;
  }
  void respawn(coord_t pos_, coord_t v_, double initvel) {
    a.x = 0;
    a.y = 100.0;
    v = v_;
    if ((v.length() == 0.0) && (initvel != 0.0))
      throw std::invalid_argument("vector should have some length!");
    if (v.length() == 0.0)
      v = v * (1.0 / v.length()) * initvel;
    pos = pos_;
  }

  void update(int df) {
    double dt = (df * 0.001);
    pos = pos + v * dt + v * a * (dt * dt) * 0.5;
    v = v + a * dt;
  }

  bool check_collision(const coord_t &shot_pos) {
    return (shot_pos - pos).length() < 16;
  }

  void draw() {
    auto p = pos;
    p.x -= anim->frames.at(0).w >> 1;
    p.y -= anim->frames.at(0).h >> 1;
    anim->draw(p, v);
  }
};

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

class duck_gun_t {
public:
  int time_to_next_duck; // in milliseconds
  std::uniform_int_distribution<> duck_timer_dist;
  std::uniform_real_distribution<> duck_dir_dist;
  std::mt19937 gen;
  duck_gun_t() {
    std::random_device rd;
    gen = std::mt19937(rd());
    duck_timer_dist = std::uniform_int_distribution<>(100, 1000);
    duck_dir_dist = std::uniform_real_distribution<>(0, 320);
    time_to_next_duck = duck_timer_dist(gen);
  }
  void update(int df, std::function<void(duck_t &)> on_new_duck) {
    time_to_next_duck -= df;
    if (time_to_next_duck <= 0) {
      coord_t position = {duck_dir_dist(gen), 230};
      coord_t target = {duck_dir_dist(gen), 50};
      duck_t new_duck;
      new_duck.respawn(position, target - position, 200);
      on_new_duck(new_duck);
      time_to_next_duck += duck_timer_dist(gen);
    }
  }
};

class player_t {
public:
  crosshair_t crosshair;
  coord_t mouse_move_direction;
  int points;
  int missed;
  std::list<coord_t> shots;
  std::shared_ptr<SDL_Texture> ducks_texture;
  std::vector<SDL_Rect> digits;

  player_t() {}
  player_t(std::shared_ptr<SDL_Texture> ducks_texture_) {
    ducks_texture = ducks_texture_;
    points = 0;
    missed = 0;
    mouse_move_direction = {0, -1.0};
    for (int i = 131; i < 131 + 8 * 6; i += 8)
      digits.push_back({i, 128, 8, 8});
    for (int i = 131; i < 131 + 8 * 4; i += 8)
      digits.push_back({i, 136, 8, 8});
  }
  void draw_digit(SDL_Renderer *renderer, coord_t p, int n, int offset = 0) {
    SDL_Rect dest = {(int)p.x, (int)p.y, digits.at(n).w, digits.at(n).h};
    auto srcr = digits.at(n);
    srcr.y += offset;
    SDL_RenderCopy(renderer, ducks_texture.get(), &srcr, &dest);
  }

  void draw(SDL_Renderer *renderer) {
    int p = points;
    for (int i = 9; i > 0; i--) {
      draw_digit(renderer, {10.0 + i * 10, 10.0}, p % 10);
      p = p / 10;
    }
    p = missed;
    for (int i = 9; i > 0; i--) {
      draw_digit(renderer, {10.0 + i * 10, 20.0}, p % 10, 16);
      p = p / 10;
    }
    crosshair.draw(renderer);
  }
};
class game_engine_t {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  std::vector<std::shared_ptr<SDL_Texture>> bg_layers;
  std::shared_ptr<spritesheet_t> duck_animation;

  std::vector<duck_t> ducks; // ducks on the move
  bool game_active = true;

  std::shared_ptr<client_t> net_client;

public:
  player_t player;
  game_engine_t(std::vector<std::string> args) {
    if (args.size() >= 2) {
       net_client = std::make_shared<client_t>(args.at(0), args.at(1), (args.size() > 2)?args.at(2):"9921");
    }

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

    bg_layers = {load_img(renderer, "background.png"),
                 load_img(renderer, "grass1.png"),
                 load_img(renderer, "grass2.png")};
    duck_animation = std::make_shared<spritesheet_t>(renderer, "dh_duck.png",
                                                     SDL_Rect{0, 0, 36, 40}, 3,
                                                     300, 1, 0x0ffa5efa3);
    player = player_t(duck_animation->tex);
    player.crosshair =
        crosshair_t(load_img(renderer, "crosshair.png"), 160, 100);
  }

  void game_loop() {
    std::vector<grass_t> grass;
    grass.push_back(grass_t(bg_layers[1], 0, 200, 10, 0));
    grass.push_back(grass_t(bg_layers[2], 0, 210, 13.123, 0));

    double dt = 0; // przyrost czasu w sekundach
    long int frame_number = 0;
    long int df = 0; // przyrost ramek/milisekund

    if (net_client.get()) {
      net_client->start_game();
    }

    auto prev_tick = SDL_GetTicks();
    duck_gun_t duck_gun;

    while (game_active && (player.missed < 10)) {
      // petla zdarzen
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
          game_active = false;
          break;
        case SDL_MOUSEMOTION:
          player.crosshair.pos.x = event.motion.x;
          player.crosshair.pos.y = event.motion.y;

          if (!((event.motion.xrel == 0) && (event.motion.yrel == 0))) {
            player.mouse_move_direction.x =
                player.mouse_move_direction.x * 0.7 + event.motion.xrel * 0.3;
            player.mouse_move_direction.y =
                player.mouse_move_direction.y * 0.7 + event.motion.yrel * 0.3;
          }

          break;
        case SDL_MOUSEBUTTONDOWN:
          if (event.button.button == SDL_BUTTON_LEFT) {
            player.shots.push_back(player.crosshair.pos);
          }
          break;
        }
      }
      // fizyka

      if (dt > 0.0) {
        for (auto &g : grass)
          g.update(dt);

        for (auto &duck : ducks)
          duck.update(df);
        duck_animation->update(df);
        duck_gun.update(df, [this](auto &duck) {
          SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "adding duck %f %f %f %f!!",
                      duck.pos.x, duck.pos.y, duck.v.x, duck.v.y);
          SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ducks %d!!", ducks.size());
          duck.anim = duck_animation;
          ducks.push_back(duck);
        });
        int prev_points = player.points;
        ducks.erase(std::remove_if(ducks.begin(), ducks.end(),
                                   [this](duck_t &d) {
                                     for (auto &shotpos : player.shots)
                                       if (d.check_collision(shotpos)) {
                                         player.points += 1;
                                         SDL_LogInfo(
                                             SDL_LOG_CATEGORY_APPLICATION,
                                             "points: %d", player.points);
                                         return true;
                                       }
                                     return d.pos.y > 400.0;
                                   }),
                    ducks.end());
        if (player.shots.size() > 0)
          if (prev_points == player.points)
            player.missed++;
        player.shots.clear();
        // SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ducks %d", ducks.size());
      }
      // grafika
      SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, bg_layers.at(0).get(), NULL, NULL);
      for (auto &g : grass)
        g.draw(renderer);

      player.draw(renderer);

      for (auto &duck : ducks)
        duck.draw();
      SDL_RenderPresent(renderer);
      auto new_tick = SDL_GetTicks();
      dt = (new_tick - prev_tick) / 1000.0;
      frame_number += (df = (new_tick - prev_tick));
      prev_tick = new_tick;
    }
  }
  ~game_engine_t() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
  }
};

int main(int argc, char *argv[]) {
  using namespace std;
  game_engine_t engine(std::vector<std::string>(argv+1, argv + argc));
  engine.game_loop();

  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Your score is %d",
              engine.player.points);

  return 0;
}
