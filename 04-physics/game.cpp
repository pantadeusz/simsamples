#include <SDL.h>

#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>
#include <array>
#include <random>
#include <list>


#include <any>

// std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 random_generator((std::random_device())()); //Standard mersenne_twister_engine seeded with rd()


using position_t = std::array<double, 2>; ///< 2d graphics and physics

position_t operator+(const position_t &a, const position_t &b) {
    return {a[0]+b[0],a[1]+b[1]};
}
position_t operator-(const position_t &a, const position_t &b) {
    return {a[0]-b[0],a[1]-b[1]};
}
position_t operator*(const position_t &a, const double &b) {
    return {a[0]*b,a[1]*b};
}
position_t operator/(const position_t &a, const double &b) {
    return {a[0]/b,a[1]/b};
}


class particle_t {
public:
    position_t p;  // pozycja
    position_t v;  // predkosc
    position_t a;  // przyspieszenie

    unsigned long int id;

    double ttl;           // czas zycia (w sekundach)

    particle_t() {
        static unsigned long int next_id = 0;
        p = {0,0};
        v = {0,0};
        ttl = 0;
        id = next_id++;
    };

    particle_t update(const double dt) const {
        particle_t updated = *this;
        updated.p = p + v * dt + a*dt*dt/2.0;
        updated.v = v + a*dt;
        updated.a = a;
        updated.ttl = ttl-1;
        return updated;
    }
};

std::vector < particle_t > update_particle_ts(std::vector < particle_t > const &  particle_ts, double dt) {
    std::vector < particle_t > ret_particle_ts;
    for (auto p: particle_ts) {
        auto new_p = p.update(dt);
        if (new_p.ttl > 0)
            ret_particle_ts.push_back(new_p);
    }
    return ret_particle_ts;
}

std::vector< particle_t > generate_explosion(position_t p, int n, double power, position_t accel, int max_ttl) {
    std::normal_distribution<double> distrib;
    std::uniform_real_distribution<double> angle_distr;
    std::uniform_int_distribution<int> ttl_distribution(1,std::max(2,max_ttl));
    std::vector< particle_t > ret;
    for (int i = 0; i < n; i++) {
        particle_t particle_t;
        particle_t.p = p;
        double v0 = distrib(random_generator)*power;
        double a = angle_distr(random_generator)*M_PI*2.0;
        particle_t.v = {std::cos(a)*v0,std::sin(a)*v0};
        particle_t.a = accel;
        particle_t.ttl = ttl_distribution(random_generator);;
        ret.push_back(particle_t);
    }
    return ret;
}

class game_events_t {
public:
    uint64_t timestamp;
    bool quit;
    std::vector<std::pair<double,position_t>> explosions_coordinates;
};

class game_state_t {
public:
    double fps;
    double dt;
    uint64_t game_tick;


    std::vector < particle_t > particle_ts;

    game_state_t() = default;

    game_state_t(double fps) {
        this->fps=fps;
        dt=1.0/fps;
        uint64_t game_tick = 0;
    };

    static game_state_t physics(const game_state_t &gs, const game_events_t &events) {
        game_state_t game_state = gs;

        for (auto [power, p]: events.explosions_coordinates) {
            std::cout << "power: " << power <<  " at " << p[0] << " " << p[1] <<std::endl;
            auto new_explosion = generate_explosion(p, 500, 100*power+1, {0,20}, 200);
            game_state.particle_ts.insert(game_state.particle_ts.end(), new_explosion.begin(), new_explosion.end());
        }
        game_state.particle_ts = update_particle_ts(game_state.particle_ts, game_state.dt);
        game_state.game_tick++;
        return game_state;
    }
};


game_events_t get_all_events(const uint64_t timestamp) {
    SDL_Event e;
    static std::map<uint64_t,uint64_t> button_down_moments;

    game_events_t ret_events;
    ret_events.timestamp = timestamp;
    ret_events.quit = false;
    while( SDL_PollEvent( &e ) != 0 ) {
        if( e.type == SDL_QUIT ) {
            ret_events.quit = true;
        } else if (e.type == SDL_KEYDOWN) {
        } else if (e.type == SDL_KEYUP) {
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            button_down_moments[e.button.button] = e.button.timestamp;
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            std::cout << e.button.x << " " << e.button.y << std::endl;
            position_t p = {(double)e.button.x, (double)e.button.y};
            ret_events.explosions_coordinates.push_back({(((double)(e.button.timestamp - button_down_moments[e.button.button] ))/1000.0), p});
        }
    }
    return ret_events;
}

class graphics_t {
    SDL_Window *window;
    SDL_Renderer *renderer;
public:
    graphics_t() {
        SDL_Init( SDL_INIT_EVERYTHING );
        SDL_CreateWindowAndRenderer(800, 600, SDL_WINDOW_RESIZABLE, &window, &renderer);
        SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, 0);
        SDL_RenderSetLogicalSize(renderer, 320, 200);
    }

    virtual ~graphics_t() {
        SDL_DestroyWindow( window );
        SDL_Quit();
    }

    void draw(const game_state_t & game_state) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        for (auto particle_t: game_state.particle_ts) {
            SDL_SetRenderDrawColor(renderer, std::min((int)255,(int)particle_t.ttl*4), std::min((int)255,(int)particle_t.ttl), std::min((int)255,(int)particle_t.ttl), 255);
            SDL_RenderDrawPoint(renderer, particle_t.p[0], particle_t.p[1]);
        }
        SDL_RenderPresent(renderer);
    }
};

int main( int argc, char* args[] ) {
    using namespace std::chrono;
    using namespace std;
    graphics_t graphics;

    game_state_t game_state(60.0);
    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
    while (true) {
        // zdarzenia
        auto events = get_all_events(game_state.game_tick);
        // physics
        game_state_t new_game_state = game_state_t::physics(game_state, events);
        game_state = new_game_state; // update state
        // timer
        bool skip_frame = false;
        auto next_time = current_time + microseconds ((long long int)(game_state.dt*1000000.0));
        if (next_time < std::chrono::steady_clock::now()) {
            skip_frame = true;
        } else {
            std::this_thread::sleep_until(next_time);
            skip_frame = false;
        }
        current_time = next_time;

        if (!skip_frame) {
            graphics.draw(game_state);
        }
        if (events.quit) break;
    }


    return 0;
}
