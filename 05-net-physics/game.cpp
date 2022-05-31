#include "tp_args.hpp"

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
#include <tuple>
#include <any>


#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>

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

    uint64_t id;

    double ttl;           // czas zycia

    int owner;

    particle_t() {
        static uint64_t next_id = 0;
        p = {0,0};
        v = {0,0};
        ttl = 0;
        id = next_id++;
        owner = 0;
    };

    /**
     * @brief
     *
     * @param dt
     * @return particle_t
     */
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

std::vector< particle_t > generate_explosion(position_t p, int owner, int n, double power, position_t accel, int max_ttl) {
    std::normal_distribution<double> distrib;
    std::uniform_real_distribution<double> angle_distr;
    std::uniform_int_distribution<int> ttl_distribution(1,std::max(2,max_ttl));
    std::vector< particle_t > ret;
    double a0 = angle_distr(random_generator)*M_PI*2;
    for (int i = 0; i < n; i++) {
        particle_t particle;
        particle.p = p;
        double v0 = std::abs(distrib(random_generator)*power);
        double a = a0 + angle_distr(random_generator)*M_PI/2;//*2.0;
        particle.v = {std::cos(a)*v0,std::sin(a)*v0};
        particle.a = accel;
        particle.ttl = ttl_distribution(random_generator);
        particle.owner = owner;
        ret.push_back(particle);
    }
    return ret;
}

class game_events_t {
public:
    uint64_t timestamp;
    bool quit;
    std::vector<std::tuple<double,int,position_t>> new_explosions;
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

        for (auto [power, id, p]: events.new_explosions) {
            std::cout << "bam: " << power <<  " at " << p[0] << " " << p[1] <<std::endl;
            auto new_explosion = generate_explosion(p, id, 500, 100*power+1, {0,20}, 200);
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
            ret_events.new_explosions.push_back({(((double)(e.button.timestamp - button_down_moments[e.button.button] ))/1000.0), e.button.button, p});
        }
    }
    return ret_events;
}

/**
 * @brief Graphics engine implementation
 *
 * it requires knowledge of game_t class
 *
 */
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
            SDL_SetRenderDrawColor(renderer,
                                   std::min((int)255,(int)particle_t.ttl*2*((1+particle_t.owner) % 3)),
                                   std::min((int)255,(int)particle_t.ttl*2*((2+particle_t.owner) % 3)),
                                   std::min((int)255,(int)particle_t.ttl*2*((3+particle_t.owner) % 3)), 255);
            SDL_RenderDrawPoint(renderer, particle_t.p[0], particle_t.p[1]);
        }
        SDL_RenderPresent(renderer);
    }
};


int run_game_main( int argc, char* args[] ) {

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


int bind_socket(unsigned int port_nr) {
    if (port_nr < 0)
        port_nr = 9922;
    int s = socket(AF_INET6, SOCK_DGRAM, 0);
    if (s < 0) throw std::invalid_argument("could not create socket");
    const int off = 0;
    if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off))) {
        close(s);
        throw std::invalid_argument("could not set options for IPV6_V6ONLY off");
    }
    {   // set nonblocking
        int flags = fcntl(s, F_GETFL, 0);
        fcntl(s, F_SETFL, flags | O_NONBLOCK);
    }
    struct sockaddr_in6 addr;
    bzero(&addr, sizeof(addr)); // bind every available interface
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port_nr);
    socklen_t alen = sizeof(addr);
    if (bind(s, (struct sockaddr *)(&addr), alen)) {
        close(s);
        throw std::invalid_argument("could not bind selected address");
    }
    return s;
}


using net_addr_t = std::pair<struct sockaddr_storage,socklen_t>;
// operator that allows for comparing two adresses.
bool operator<(const net_addr_t & a, const net_addr_t &b) {
    for (int i = 0; i < std::min(a.second, b.second); i++) {
        if (((char *)&(a.first))[i] < ((char *)&(b.first))[i]) return true;
    }
    if (a.second < b.second) return true;
    return false;
}


std::vector<net_addr_t> find_addresses(const std::string addr_txt,
                                       const int port, int ai_family = AF_UNSPEC) {
    const std::string port_txt = std::to_string(port);
    struct addrinfo hints;
    std::vector<net_addr_t> ret_addresses;
    bzero((char *)&hints, sizeof(struct addrinfo));
    hints.ai_family = ai_family;
    hints.ai_socktype = SOCK_DGRAM;

    struct addrinfo *addr_p;
    int err = getaddrinfo(addr_txt.c_str(), port_txt.c_str(), &hints, &addr_p);
    if (err) {
        throw std::invalid_argument( "getaddrinfo error: " + std::to_string(err) );
    }
    for (struct addrinfo *rp = addr_p; rp != NULL; rp = rp->ai_next) {
        struct sockaddr_storage a;
        socklen_t l;
        if (rp->ai_addrlen < sizeof(a)) {
            memcpy(&a, rp->ai_addr, rp->ai_addrlen); // warning!
            l = rp->ai_addrlen;
            ret_addresses.push_back({a,l});
        }
    }
    freeaddrinfo(addr_p);
    return ret_addresses;
}

class message_t {
public:
// data size is simplified - always 128 bytes
    std::array<char,128> data;
    char get_type() const {
        return data[0];
    }
    void set_type(char t) {
        data[0] = t;
    }
    char *get_data() {
        return data.data()+1;
    }

};

class hello_message_t: public message_t {
public:
    hello_message_t(std::string name) {
        this->set_type(1);
    };
};

class game_server_t {
public:
    int udp_socket;
    game_server_t( int port) {
        udp_socket = bind_socket(port);
    }
    virtual ~game_server_t() {
        close(udp_socket);
    }

    int data_exchange() {
        while (true) {
            struct sockaddr_storage a1,a2;
            struct sockaddr_storage peer_addr;
            socklen_t peer_addr_len = sizeof(struct sockaddr_storage);

            char buffer[128];
            ssize_t rsize = recvfrom(udp_socket, buffer, 128, MSG_DONTWAIT,
                                     (struct sockaddr *)&peer_addr, &peer_addr_len);
            if (rsize < 128) break; // get all messages.
            net_addr_t addr = {peer_addr, peer_addr_len};
            if (buffer[0] == 1) {
                // send back our introduction
                sendto(udp_socket, buffer, 128, 0, (struct sockaddr *)&addr.first, addr.second);
                std::cerr << "hello message...." << ((int)buffer[0])<< std::endl;
            } else {
                std::cerr << "unknown message...." << ((int)buffer[0])<< std::endl;
            }
        }
        return 0;
    }

    int run() {
        while (true) {
            data_exchange();
            sleep(1);
        }
    }
};

int run_game_server(int port) {
    game_server_t server ( port);

    return server.run();
}

int run_game_client(std::string player_name, std::string hostname, int port) {
    int s = -1;
    for (int myport = port+1; myport < myport+128; myport++) {
        try {
            s  = bind_socket(myport);
            break;
        } catch (std::invalid_argument e) {
            std::cout << e.what() << ": port " << myport << " taken, trying another" << std::endl;
        }
    }
    if (s == -1) throw std::invalid_argument("cannot bind port");
    auto addresses = find_addresses(hostname, port);
    std::cout << "had adresses, now try to connect" << std::endl;
    for (auto [s_addr,s_len] : addresses) {
        std::cout <<"hello sending.." << std::endl;
        hello_message_t message(player_name);
        message_t pinged_message;
        sendto(s, message.data.data(), message.data.size(), 0, (struct sockaddr *)&s_addr,
               s_len);

        std::cout <<"hello getting.." << std::endl;
        struct sockaddr_storage peer_addr;
        socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
        ssize_t rsize = recvfrom(s,
                                 pinged_message.data.data(), pinged_message.data.size(), 0,
                                 (struct sockaddr *)&peer_addr, &peer_addr_len);
        std::cout << "ok" << std::endl;
    }
    return 0;
}

int main( int argc, char* argv[] ) {
    using namespace tp::args;
    auto help = arg( argc, argv, "help", false, "show help screen" );
    auto scan = arg( argc, argv, "scan", false, "search for server" );
    auto server = arg( argc, argv, "server", false, "runs in server mode" );
    auto client = arg( argc, argv, "client", false, "runs in client mode" );
    auto host = arg( argc, argv, "host", "127.0.0.1", "server address" );
    auto port = arg( argc, argv, "port", 12312, "server port" );
    auto playername = arg(argc, argv, "playername", "nameless", "set player name - must be unique");
    if ( help ) {
        std::cout << "Simple client-server demo on UDP protocol" << std::endl;
        args_info( std::cout );
        return 0;
    }
    if (server) {
        return run_game_server(port);
    } else if (client) {
        return run_game_client(playername, host,port);
//        return run_game_main(argc, argv);
    }
}

