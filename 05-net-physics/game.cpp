#include "tp_args.hpp"

#include "net_messages.hpp"

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
    std::vector<net::explosion_t> new_explosions;
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

        for (auto explosion: events.new_explosions) {
            auto new_explosion = generate_explosion(explosion.p, explosion.id, 500, 100*explosion.power+1, {0,20}, 200);
            game_state.particle_ts.insert(game_state.particle_ts.end(), new_explosion.begin(), new_explosion.end());
        }
        game_state.particle_ts = update_particle_ts(game_state.particle_ts, game_state.dt);
        game_state.game_tick++;
        return game_state;
    }
};


game_events_t get_all_events(const uint64_t timestamp, net::player_t player) {
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
            net::explosion_t explosion;
            std::cout << e.button.x << " " << e.button.y << std::endl;
            explosion.p = {(double)e.button.x, (double)e.button.y};
            explosion.power = (((double)(e.button.timestamp - button_down_moments[e.button.button] ))/1000.0);
            explosion.id = player.id;
            explosion.game_tick = timestamp;
            ret_events.new_explosions.push_back(explosion);
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



int bind_socket(unsigned int port_nr) {
    if (port_nr < 0)
        port_nr = 9922;
    int client_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (client_socket < 0) throw std::invalid_argument("could not create socket");
    const int off = 0;
    if (setsockopt(client_socket, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off))) {
        close(client_socket);
        throw std::invalid_argument("could not set options for IPV6_V6ONLY off");
    }
    {   // set nonblocking
        int flags = fcntl(client_socket, F_GETFL, 0);
        fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
    }
    struct sockaddr_in6 addr;
    bzero(&addr, sizeof(addr)); // bind every available interface
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port_nr);
    socklen_t alen = sizeof(addr);
    if (bind(client_socket, (struct sockaddr *)(&addr), alen)) {
        close(client_socket);
        throw std::invalid_argument("could not bind selected address");
    }
    return client_socket;
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




class game_server_t {
public:
    int udp_socket;

    std::map<net_addr_t, net::player_t> players;

    game_server_t( int port) {
        udp_socket = bind_socket(port);
    }
    virtual ~game_server_t() {
        close(udp_socket);
    }

    int find_free_player_id() {
        for (int i = 0; i < players.size()+1; i++) {
            int found = -1;
            for (auto [addr, player] : players) {
                auto [type,id, name] = player;
                if (id == i) {
                    found = id;
                    break;
                }
            }
            if (found == -1) return i;
        }
        return -1;
    }

    static void handle_payer_t(game_server_t *pthis, net::message_t message, net_addr_t addr) {
        // send back our introduction
        net::player_t value = message.player;
        std::cerr << "hello or goodbye message.... " << value.id << " " << value.player_name  << std::endl;
        if (value.id >= 0) {
            /// goodbye message
            if (pthis->players.count(addr)) {
                pthis->players.erase(addr);
                std::cerr << "goodbye message>>>> " << value.id << " " << value.player_name  << std::endl;
            } else {
                std::cerr << "player not found but wants to disconnect" << std::endl;
            }
        } else {
            // hello message
            value.id = pthis->find_free_player_id();
            pthis->players[addr] = value;
            message.player = value;
            sendto(pthis->udp_socket, message.data, sizeof(message.data), 0, (struct sockaddr *)&addr.first, addr.second);
            std::cerr << "hello message>>>> " << value.id << " " << value.player_name  << std::endl;
        }
    }

    static void handle_explosion_t(game_server_t *pthis, net::message_t message, net_addr_t from_addr) {
        int from_id = pthis->players[from_addr].id;
        for (auto [addr, player] : pthis->players) {
            if (player.id != from_id) {
                message.explosion.id = from_id;
                sendto(pthis->udp_socket, message.data, sizeof(message.data), 0, (struct sockaddr *)&addr.first, addr.second);
            }
        }
    }

    void handle_incoming_message(net::message_t message, net_addr_t addr) {
        if (net::MESSAGE_PLAYER == message.type) {
            handle_payer_t(this, message, addr);
        } else if (net::MESSAGE_EXPLOSION == message.type) {
            handle_explosion_t(this, message, addr);
        } else {
            std::cerr << "unknown message...." << message.type << std::endl;
        }

    }

    int data_exchange() {
        while (true) {
            net_addr_t addr = {{}, sizeof(struct sockaddr_storage)};
            net::message_t buffer;
            ssize_t rsize = recvfrom(udp_socket, buffer.data, sizeof(buffer.data), MSG_DONTWAIT,
                                     (struct sockaddr *)&addr.first, &addr.second);
            if (rsize < 128) break; // get all messages.

            handle_incoming_message(buffer, addr);
        }
        return 0;
    }

    int run() {
        using namespace std::chrono_literals;
        while (true) {
            data_exchange();
            std::this_thread::sleep_for(10ms);
        }
    }
};

int run_game_server(int port) {
    game_server_t server ( port);

    return server.run();
}

class game_client_t {
public:
    net::player_t local_player;
    net_addr_t server_addr;
    int client_socket; // socket

    std::map<int,net::player_t> players;

    game_events_t get_all_network_events(game_events_t events) {
        ssize_t rsize = 0;
        net::message_t buf;

        while ((rsize = recvfrom(client_socket, buf.data, sizeof(buf.data), 0, NULL, 0)) == sizeof(net::message_t))
        {
            if (buf.type == net::MESSAGE_EXPLOSION) {
                events.new_explosions.push_back(net::explosion_t::from_message(buf.explosion));
            } else {
                std::cerr << "unknown message" << std::endl;
            }
        }
        return events;
    }
    void send_events(const game_events_t &events) {
        auto [s_addr,s_len] =server_addr;
        for (auto e : events.new_explosions) {
            net::message_t buf = {explosion:e.to_message()};

            sendto(client_socket, buf.data, sizeof(buf.data), 0,
                   (struct sockaddr *)&s_addr, s_len);
        }
    }

    void handshake(std::string player_name, std::string hostname, int port) {
        using namespace std::chrono_literals;
        client_socket = -1;
        for (int myport = port+1; myport < myport+128; myport++) {
            try {
                client_socket  = bind_socket(myport);
                break;
            } catch (std::invalid_argument e) {
                std::cout << e.what() << ": port " << myport << " taken, trying another" << std::endl;
            }
        }
        if (client_socket == -1) throw std::invalid_argument("cannot bind port");
        auto addresses = find_addresses(hostname, port);
        std::cout << "had adresses, now try to connect" << std::endl;
        for (auto [s_addr,s_len] : addresses) {
            strcpy(local_player.player_name, player_name.c_str());
            local_player.id = -1;
            local_player.type = net::MESSAGE_PLAYER;
            net::message_t buf;
            buf.player = local_player;
            sendto(client_socket, buf.data, sizeof(buf.data), 0, (struct sockaddr *)&s_addr, s_len);

            std::cout <<"hello getting.." << std::endl;
            struct sockaddr_storage peer_addr;
            socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
            for (int i = 0; i < 5; i++) {
                ssize_t rsize = recvfrom(client_socket, buf.data, sizeof(buf.data), 0,
                                         (struct sockaddr *)&peer_addr, &peer_addr_len);
                if (rsize == 128) break;
                std::this_thread::sleep_for(1s);
            }
            local_player = buf.player;
            if ((local_player.id != -1) && (local_player.type == net::MESSAGE_PLAYER)) {
                std::cout << "connected: " << local_player.id << " " << local_player.player_name << std::endl;
                server_addr = {s_addr,s_len};
                break;
            } else {
                std::cout << "could not connect... timeout" << std::endl;
            }
        }

    }

    void goodbye() {
        if (local_player.id != -1) {
            auto [s_addr,s_len] =server_addr;
            net::message_t buf = {player:local_player};
            sendto(client_socket, buf.data, sizeof(buf.data), 0, (struct sockaddr *)&s_addr,               s_len);
        }

    }

    game_client_t(std::string player_name, std::string hostname, int port) {
        handshake(player_name,hostname, port);

    }
};

int run_game_client(std::string player_name, std::string hostname, int port) {
    game_client_t game_client(player_name, hostname, port);



    using namespace std::chrono;
    using namespace std;
    graphics_t graphics;

    game_state_t game_state(60.0);

    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
    while (true) {
        // zdarzenia
        auto events = get_all_events(game_state.game_tick, game_client.local_player);
        game_client.send_events(events);
        events = game_client.get_all_network_events(events);
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

    // send goodbye message
    game_client.goodbye();
    return 0;

}

int main( int argc, char* argv[] ) {
    using namespace tp::args;
    auto help = arg( argc, argv, "help", false, "show help screen" );
    auto scan = arg( argc, argv, "scan", false, "search for server" );
    auto server = arg( argc, argv, "server", false, "runs in server mode" );
    auto client = arg( argc, argv, "client", false, "runs in client mode" );
    auto host = arg( argc, argv, "host", std::string("127.0.0.1"), "server address" );
    auto port = arg( argc, argv, "port", 12312, "server port" );
    auto player_name = arg(argc, argv, "player_name", std::string("nameless"), "set player name - must be unique");
    if ( help ) {
        std::cout << "Simple client-server demo on UDP protocol" << std::endl;
        args_info( std::cout );
        return 0;
    }
    if (server) {
        return run_game_server(port);
    } else if (client) {
        return run_game_client(player_name, host,port);
//        return run_game_main(argc, argv);
    }
}

