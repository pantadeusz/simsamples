/**
 * This is the simple hello world for SDL2.
 *
 * You need C++14 to compile this.
 */

#include <SDL2/SDL.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <lodepng.h>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

// check for errors
#define errcheck(e)                                                            \
  {                                                                            \
    if (e)                                                                     \
      throw std::invalid_argument(SDL_GetError());                             \
  }

namespace gameengine {
using namespace std; // only in this namespace!!

///////////////////////////////////////////////////////////////////////////////
///////////////////     STAŁE GRY                    //////////////////////////
///////////////////////////////////////////////////////////////////////////////

const int tile_size[] = {32, 32};

///////////////////////////////////////////////////////////////////////////////
///////////////////     STRUKTURY                    //////////////////////////
///////////////////////////////////////////////////////////////////////////////

using position_t = std::array<double, 2>;

enum event_enum { NONE, QUIT };
struct player_intention_t {
  position_t move;
};

struct player_t {
  position_t acceleration;
  position_t velocity;
  position_t position;
  player_intention_t intention;

  std::shared_ptr<SDL_Texture> texture;
};

struct game_map_t {
  std::vector<std::string> t; // opis kafelkow
  // # przeszkoda
  // . puste pole
  // , ziemia
};

struct game_state_t {
  // gracze
  std::vector<player_t> players;
  // aktualna klatka
  int frame;

  shared_ptr<game_map_t> world;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////   PRZECIĄŻONE OPERATORY          //////////////////////////
///////////////////////////////////////////////////////////////////////////////
position_t operator/(const position_t &e, double s) {
  return {e[0] / s, e[1] / s};
}

position_t operator*(const position_t &e, double s) {
  return {e[0] * s, e[1] * s};
}

position_t operator+(const position_t &e, const position_t &s) {
  return {e[0] + s[0], e[1] + s[1]};
}

position_t operator-(const position_t &e, const position_t &s) {
  return {e[0] - s[0], e[1] - s[1]};
}

position_t operator*(const position_t &e, const position_t &s) {
  return {e[0] * s[0], e[1] * s[1]};
}

/**
 * @brief Długość wektora
 */
double operator~(const position_t &e) {
  return std::sqrt(e[0] * e[0] + e[1] * e[1]);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////   FUNKCJE                        //////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct hardware_objects_t {
  SDL_Renderer *renderer;
  SDL_Window *window;
};

/**
 * @brief przygotowanie renderera - na tym bedziemy rysowali
 */
shared_ptr<hardware_objects_t> init_hardware_subsystems(int width, int height,
                                                        bool fscreen) {
  // inicjalizacja SDL tylko jesli jeszcze nie byl zainicjowany
  errcheck(SDL_Init(SDL_INIT_EVERYTHING) != 0);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
  auto window = SDL_CreateWindow(
      "Textures example", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, width, height,
      SDL_WINDOW_SHOWN | (fscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
  errcheck(window == nullptr);
  auto renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  errcheck(renderer == nullptr);

  hardware_objects_t *objects = new hardware_objects_t;
  objects->renderer = renderer;
  objects->window = window;

  // opakowujemy wszystko w smartpointer dzieki czemu nie musimy pamietac o
  // zwalnianiu zasobow
  return shared_ptr<hardware_objects_t>(objects, [](auto o) {
    SDL_DestroyRenderer(o->renderer);
    SDL_DestroyWindow(o->window);
    SDL_Quit();
    delete o;
  });
}

/**
 * @brief Ładuje teksturę z pliku png
 *
 */
std::shared_ptr<SDL_Texture> load_texture(SDL_Renderer *renderer,
                                          const std::string fname) {
  // SDL_Surface *bmp = SDL_LoadBMP( fname.c_str() );
  std::vector<unsigned char> image;
  unsigned width, height;
  errcheck(lodepng::decode(image, width, height, fname));

  SDL_Surface *bitmap = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32,
                                                       SDL_PIXELFORMAT_RGBA32);
  std::copy(image.begin(), image.end(), (unsigned char *)bitmap->pixels);
  errcheck(bitmap == nullptr);

  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, bitmap);

  errcheck(tex == nullptr);
  SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  std::shared_ptr<SDL_Texture> texture(
      tex, [](SDL_Texture *ptr) { SDL_DestroyTexture(ptr); });
  SDL_FreeSurface(bitmap);
  return texture;
}

/**
 * @brief Ładowanie planszy oraz przygotowanie obiektu gracza
 *
 */
game_state_t load_level() {
  game_state_t game_state;
  game_state.players.push_back({.acceleration = {0, 0},
                                .velocity = {0, 0},
                                .position = {1, 1},
                                .intention = {0, 0},
                                .texture = nullptr});
  // ładowanie mapy
  game_state.world = make_shared<game_map_t>();
  std::ifstream t("data/level1.txt"); // załadujmy plik
  std::istringstream iss(std::string((std::istreambuf_iterator<char>(t)),
                                     std::istreambuf_iterator<char>()));
  game_state.world->t =
      std::vector<std::string>((std::istream_iterator<std::string>(iss)),
                               std::istream_iterator<std::string>());
  return game_state;
}

/**
 * @brief przetwarzanie wszystkich zdarzeń oraz pobieranie "intencji" ruchu
 * poszczególnych graczy
 */
std::map<int, player_intention_t>
process_input(shared_ptr<hardware_objects_t> hw,
              std::map<event_enum, std::function<void()>> event_handlers) {

  std::map<SDL_Scancode, std::function<std::pair<int, player_intention_t>()>>
      keyboard_mapping;

  std::map<int, player_intention_t> intentions;
  // pobranie zdarzen sdl
  SDL_Event event;
  while (SDL_PollEvent(&event)) { // pętla sprawdzająca wszystkie zdarzenia
    event_enum evnt = NONE;
    switch (event.type) {
    case SDL_QUIT:
      evnt = QUIT;
      break;
    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_ESCAPE)
        evnt = QUIT;
      break;
    }
    if (event_handlers.count(evnt))
      event_handlers[evnt]();
  }
  // obslugujemy klawisze
  auto kstate = SDL_GetKeyboardState(NULL);
  if (kstate[SDL_SCANCODE_LEFT])
    intentions[0].move[0] -= 1.0;
  if (kstate[SDL_SCANCODE_RIGHT])
    intentions[0].move[0] += 1.0;
  if (kstate[SDL_SCANCODE_UP])
    intentions[0].move[1] -= 1.0;
  if (kstate[SDL_SCANCODE_DOWN])
    intentions[0].move[1] += 1.0;

  // jeśli nie działa przypisanie strukturalne, zastosuj
  // for (auto plr : intentions) { auto &p = plr.second;
  for (auto &[i, p] : intentions) {
    if (~p.move > 1.0)
      p.move = p.move / (~p.move); // skalujemy do jedności
  }
  return intentions;
}

/**
 * @brief Obliczenia fizyczne i obsluga wszelakich zdarzen w swiecie gry
 */
game_state_t
calculate_next_game_state(const game_state_t &previous_state,
                          std::map<int, player_intention_t> intentions,
                          double dt) {
  game_state_t ret = previous_state;
  // przetwarzamy intencje (o ile jakieś są)
  for (auto &player : ret.players) {
    player.intention = {0, 0};
  }
  for (auto &intent : intentions) {
    if (intent.first < ret.players.size())
      ret.players[intent.first].intention = intent.second;
  }

  // fizyka
  for (auto &p : ret.players) {
    auto vnorm = p.velocity;
    if (~vnorm > 0)
      vnorm = vnorm / ~vnorm;
    p.acceleration = (p.intention.move * 10.0) + ((p.velocity * p.velocity * vnorm) * -1.1);
    p.velocity = p.velocity + p.acceleration * dt;
    p.position = p.position + (p.velocity * dt) + (p.velocity * p.acceleration * dt * dt * 0.5);
  }

  // kolizje
  for (unsigned int i = 0; i < ret.players.size(); i++) {
    auto &p = ret.players[i];
    if ((p.position[0] < 0) || (p.position[0] >= ret.world->t.at(0).size())) {
      // dobicie po osi X dla krawedzi mapy
      p = previous_state.players[i];
      p.velocity[0] = p.velocity[0] * -0.6;
    } else if ((p.position[1] < 0) || (p.position[1] >= ret.world->t.size())) {
      // odbicie po osi Y dla krawedzi mapy
      p = previous_state.players[i];
      p.velocity[1] = p.velocity[1] * -0.6;
    } else {
      // kolizja z obiektem z gry
      if (ret.world->t.at(p.position[1]).at(p.position[0]) == '#') {
        //p.velocity = p.velocity * -0.1;
        auto v_one = p.velocity/~p.velocity;
        auto d_one = (p.position-position_t{0.5,0.5})-position_t{(int)(p.position[0]),(int)(p.position[1])};
        auto cp = position_t{(int)(p.position[0]),(int)(p.position[1])};
        d_one = d_one/~d_one;
        p = previous_state.players[i];
        p.velocity = ((v_one+d_one)/~(v_one+d_one))*(~p.velocity*0.9);
      }
    }
  }

  ret.frame++;
  return ret;
}

/**
 * @brief Rysowanie gracza na ekranie
 *
 */
void draw_player(SDL_Renderer *renderer, player_t &player) {
  SDL_Rect rect = {(int)((player.position[0] - 0.5) * tile_size[0]),
                   (int)((player.position[1] - 0.5) * tile_size[1]),
                   tile_size[0], tile_size[1]};
  SDL_RenderCopy(renderer, player.texture.get(), NULL, &rect);
}

/**
 * @brief Wyświetlenie wszystkiego
 */
void draw_world(SDL_Renderer *renderer, game_state_t &state) {
  SDL_SetRenderDrawColor(renderer, 255, 128, 128, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  // na początek plansza
  for (unsigned y = 0; y < state.world->t.size(); y++) {
    for (unsigned x = 0; x < state.world->t.at(0).size(); x++) {
      switch (state.world->t[y][x]) {
      case '#':
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        break;
      case ',':
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        break;
      default:
        break;
      }
      SDL_Rect rect = {(int)((x)*tile_size[0]), (int)((y)*tile_size[1]),
                       tile_size[0], tile_size[1]};
      SDL_RenderDrawRect(renderer, &rect);
    }
  }

  // następnie gracze
  for (auto p : state.players)
    draw_player(renderer, p);

  SDL_RenderPresent(renderer); // wyswietlenie backbufora
}

} // namespace gameengine

int main(int, char **) {
  using namespace std;
  using namespace std::chrono;

  auto hardware = gameengine::init_hardware_subsystems(640, 480, false);
  auto level = gameengine::load_level();
  for (auto &p : level.players)
    p.texture = gameengine::load_texture(hardware->renderer, "data/player.png");
  // auto dt = 15ms;
  milliseconds dt(15);

  steady_clock::time_point current_time = steady_clock::now();
  for (bool game_active = true; game_active;) {

    auto intentions = gameengine::process_input(
        hardware,
        {{gameengine::event_enum::QUIT, [&]() { game_active = false; }}});

    level = calculate_next_game_state(level, intentions,
                                      (double)(dt.count()) / 1000.0);

    gameengine::draw_world(hardware->renderer, level);

    this_thread::sleep_until(current_time = current_time + dt);
  }

  return 0;
}
