#include <SDL.h>

#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>
#include <array>
#include <random>

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


class Particle {
public:
    position_t p;  // pozycja
    position_t v;  // predkosc
    position_t a;  // przyspieszenie

    double ttl;           // czas zycia (w sekundach)

    Particle() {
        p = {0,0};
        v = {0,0};
        ttl = 0;
    };

    Particle update(double dt) {
        Particle updated;
        updated.p = p + v * dt + a*dt*dt/2.0;
        updated.v = v + a*dt;
        updated.a = a;
        updated.ttl = ttl-1;
        return updated;
    }
};

std::vector < Particle > update_particles(std::vector < Particle > &particles, double dt) {
    std::vector < Particle > ret_particles;
    for (auto p: particles) {
        auto new_p = p.update(dt);
        if (new_p.ttl > 0)
            ret_particles.push_back(new_p);
    }
    return ret_particles;
}

std::vector< Particle > generate_explosion(position_t p, int n, double power, position_t accel, int max_ttl) {
    std::normal_distribution<double> distrib;
    std::uniform_real_distribution<double> angle_distr;
    std::uniform_int_distribution<int> ttl_distribution(1,std::max(2,max_ttl));
    std::vector< Particle > ret;
    for (int i = 0; i < n; i++) {
        Particle particle;
        particle.p = p;
        double v0 = distrib(random_generator)*power;
        double a = angle_distr(random_generator)*M_PI*2.0;
        particle.v = {std::cos(a)*v0,std::sin(a)*v0};
        particle.a = accel;
        particle.ttl = ttl_distribution(random_generator);;
        ret.push_back(particle);
    }
    return ret;
}

int main( int argc, char* args[] ) {
    using namespace std;
    using namespace std::chrono;
    bool finishCondition = false;

    SDL_Init( SDL_INIT_EVERYTHING );
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);
    vector < Particle > particles;
    double fps=60.0; // 60FPS
    double dt=1.0/fps; // 60FPS

    steady_clock::time_point current_time = steady_clock::now(); // remember current time

    int game_tick = 0;
    int mouse_button_down = 0;
    while (!finishCondition) {
        bool skipFrame;
        std::vector<std::pair<double,position_t>> explosions_coordinates;
        // zdarzenia
        SDL_Event e;
        while( SDL_PollEvent( &e ) != 0 ) {
            if( e.type == SDL_QUIT ) {
                finishCondition = true;
            } else if (e.type == SDL_KEYDOWN) {
            } else if (e.type == SDL_KEYUP) {
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                mouse_button_down = e.button.timestamp;// game_tick;
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                std::cout << e.button.x << " " << e.button.y << std::endl;
                position_t p = {e.button.x, e.button.y};
                
                explosions_coordinates.push_back({(((double)(e.button.timestamp - mouse_button_down))/1000.0), p});
                
            }
        }

        //// physics

        for (auto [power, p]: explosions_coordinates) {
            std::cout << "power: " << power << std::endl;
            auto new_explosion = generate_explosion(p, 500, 100*power+1, {0,20}, 200);
            particles.insert(particles.end(), new_explosion.begin(), new_explosion.end());
        }
        particles = update_particles(particles, dt);

        /// timer

        auto next_time = current_time + microseconds ((long long int)(dt*1000000.0));
        if (next_time < std::chrono::steady_clock::now()) {
            skipFrame = true;
        } else {
            std::this_thread::sleep_until(next_time);
            skipFrame = false;
        }
        current_time = next_time;
        game_tick ++;

/// draw
        if (!skipFrame) {
            // grafika
            // glVertex3f(p[0], p[1], 0.0);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
            for (auto particle: particles) {
                SDL_SetRenderDrawColor(renderer, std::min((int)255,(int)particle.ttl*4), std::min((int)255,(int)particle.ttl), std::min((int)255,(int)particle.ttl), 255);
                SDL_RenderDrawPoint(renderer, particle.p[0], particle.p[1]);
            }
            SDL_RenderPresent(renderer);
        }

    }

    SDL_DestroyWindow( window );
    SDL_Quit();
    return 0;
}
