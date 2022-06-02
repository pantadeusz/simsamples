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
        double a = a0 + angle_distr(random_generator)*M_PI*2.0;
        particle.v = {std::cos(a)*v0,std::sin(a)*v0};
        particle.a = accel;
        particle.ttl = ttl_distribution(random_generator);
        particle.owner = owner;
        ret.push_back(particle);
    }
    return ret;
}


using game_events_t = std::list<net::message_t>;

class game_state_t {
public:
    uint64_t game_tick;

    std::vector < particle_t > particle_ts;

    game_state_t() = default;

    game_state_t(uint64_t game_tick_) {
        game_tick = game_tick_;
    };

    static game_state_t physics(const game_state_t &gs, const game_events_t &events) {
        game_state_t game_state = gs;

        for (auto event: events) {
            // && (event.explosion.game_tick == gs.game_tick)
            if ((event.type == net::MESSAGE_EXPLOSION) ) {
                auto explosion = event.explosion;
                auto new_explosion = generate_explosion(explosion.p, explosion.id, 500, 100*explosion.power+1, {0,20}, 200);
                game_state.particle_ts.insert(game_state.particle_ts.end(), new_explosion.begin(), new_explosion.end());
            }
        }
        game_state.particle_ts = update_particle_ts(game_state.particle_ts, net::dt);
        game_state.game_tick++;
        return game_state;
    }
};


game_events_t get_all_events(const uint64_t timestamp, net::player_t player) {
    SDL_Event e;
    static std::map<uint64_t,uint64_t> button_down_moments;

    game_events_t ret_events;
    //ret_events.quit = false;
    while( SDL_PollEvent( &e ) != 0 ) {
        if( e.type == SDL_QUIT ) {
            net::end_game_t q;
            q.game_tick = timestamp;
            q.quit = true;
            ret_events.push_back(q.m());
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
            explosion.type = net::MESSAGE_EXPLOSION;
            explosion.game_tick = timestamp;
            ret_events.push_back({explosion:explosion});
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
                                   std::min((int)255,(int)particle_t.ttl*((1+particle_t.owner) % 8)),
                                   std::min((int)255,(int)particle_t.ttl*((3+particle_t.owner) % 8)),
                                   std::min((int)255,(int)particle_t.ttl*((6+particle_t.owner) % 8)), 255);
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
    uint64_t aa = 0,bb=0;
    for (int i = 0 ; i < a.second; i++) {
        aa = (aa << 4)+((char *)&(a.first))[i];
    }
    for (int i = 0 ; i < b.second; i++) {
        bb = (bb << 4)+((char *)&(b.first))[i];
    }
    return aa < bb;
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

    uint64_t game_tick;

    std::map<uint16_t, std::pair<net::player_t,net_addr_t>> players;

    game_server_t( int port) {
        udp_socket = bind_socket(port);
        game_tick = random_generator();
    }
    virtual ~game_server_t() {
        close(udp_socket);
    }

    int find_free_player_id() {
        static int last_id = 0;
        return ++last_id;
    }

    static void handle_payer_t(game_server_t *pthis, net::message_t message, net_addr_t addr) {
        // send back our introduction
        net::player_t value = message.player;
        std::cerr << "hello or goodbye message.... " << value.id << " " << value.player_name  << std::endl;
        if (value.id >= 0) {
            /// goodbye message
            if (pthis->players.count(value.id)) {
                pthis->players.erase(value.id);
                std::cerr << "goodbye message>>>> " << value.id << " " << value.player_name  << std::endl;
            } else {
                std::cerr << "player not found but wants to disconnect" << std::endl;
            }
        } else {
            // hello message
            value.id = pthis->find_free_player_id();
            value.game_tick = pthis->game_tick;
            pthis->players[value.id] = {value,addr};
            message = value.m();
            sendto(pthis->udp_socket, message.data, sizeof(message.data), 0, (struct sockaddr *)&addr.first, addr.second);
            std::cerr << "hello message>>>> " << value.id << " " << value.player_name  << std::endl;
            for (auto [addr, player] : pthis->players) {
                std::cout << "    PLAYER: " <<player.first.id << " : " << player.first.player_name << std::endl;
            }
        }
    }

    static void handle_explosion_t(game_server_t *pthis, net::message_t message, net_addr_t from_addr) {
        int from_id = pthis->players[message.explosion.id].first.id;
        if (message.explosion.id != from_id) {
            std::cout << "ids don't match" << std::endl;
            return;
        }
        for (auto [id, player] : pthis->players) {
            //if (player.id != from_id) {
            if (message.explosion.id != from_id) std::cerr << "somehow the from_id " << from_id << " is different than that in explosion "<<message.explosion.id<<"..." << std::endl;
            message.explosion.id = from_id;
            std::cout << "   explosion to " << player.first.id << " from " << from_id << " serverTick: " << pthis->game_tick << " clientTick: " << message.explosion.game_tick  << std::endl;
            auto addr = player.second;
            sendto(pthis->udp_socket, message.data, sizeof(message.data), 0, (struct sockaddr *)&addr.first, addr.second);
            //}
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
        // send tick info if needed
        {
            std::uniform_real_distribution<double> distr;
            if (distr(random_generator) < 0.1) { // send all tick info
                net::tick_t tick;
                tick.game_tick = game_tick;
                auto message = tick.m();
                for (auto &[id, player]:players) {
                    auto addr = player.second;
                    sendto(udp_socket, message.data, sizeof(message.data), 0, (struct sockaddr *)&addr.first, addr.second);
                }
            }

        }
        return 0;
    }

    int run() {
        using namespace std::chrono_literals;
        using namespace std::chrono;
        using namespace std;
        game_tick = 0;
        std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
        while (true) {
            data_exchange(); // data exchange is less frequent than the players ticks
            auto next_time = current_time + microseconds ((long long int)(net::server_dt*1000000.0));
            if (next_time > std::chrono::steady_clock::now()) {
                std::this_thread::sleep_until(next_time);
            } else {
                std::cerr << "no delay" << std::endl;
            }
            current_time = next_time;
            game_tick += net::server_dt_multiply;
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

    game_events_t get_all_network_events() {
        game_events_t events;
        ssize_t rsize = 0;
        net::message_t buf;

        while ((rsize = recvfrom(client_socket, buf.data, sizeof(buf.data), 0, NULL, 0)) == sizeof(net::message_t))
        {
            events.push_back(buf);
        }
        return events;
    }
    void send_events(const game_events_t &events) {
        auto [s_addr,s_len] =server_addr;
        for (auto buf : events) if (buf.type == net::MESSAGE_EXPLOSION) {
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
            for (int i = 0; i < 300; i++) {
                ssize_t rsize = recvfrom(client_socket, buf.data, sizeof(buf.data), 0,
                                         (struct sockaddr *)&peer_addr, &peer_addr_len);
                if (rsize == 128) break;
                std::this_thread::sleep_for(10ms);
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

    auto clear_history = [](uint64_t game_tick, auto &history) {
        std::list<uint64_t> history_to_del;
        for (auto &[t,v] : history) {
            if (t < game_tick) history_to_del.push_back(t);
        }
        for (auto t : history_to_del) history.erase(t);
    };

    using namespace std::chrono;
    using namespace std;
    graphics_t graphics;

    std::uint64_t game_tick = game_client.local_player.game_tick;
    std::map< uint64_t,game_state_t > game_state = {{game_tick,game_state_t(game_tick)}};
    std::map< uint64_t,game_events_t > events_history;// = {{game_tick,{}}};

    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();

    for (bool game_on = true; game_on;) {
        game_tick ++;
        // zdarzenia
        const auto events = get_all_events(game_tick, game_client.local_player);
        //events_history[game_tick].insert(events_history[game_tick].end(), events.begin(), events.end());
        game_client.send_events(events);

        // find what is the oldest net event that we must process based on the network events from server
        uint64_t oldest_net_event = game_tick;
        for (auto e : game_client.get_all_network_events()) {
            if (e.type == net::MESSAGE_EXPLOSION) {
                uint64_t t = e.explosion.game_tick;
                events_history[t].push_back(e);
                oldest_net_event = std::min(t,oldest_net_event);
            } else if (e.type == net::MESSAGE_TICK) {
                game_tick = e.tick.game_tick; // synchronize to server :)
            }
        }

        // find the oldest known state from the state history
        uint64_t oldest_known_state = game_tick;
        for (auto &[t,v]:game_state) {
            if (t < oldest_known_state) oldest_known_state = t;
        }

        // if the oldest known state is latest than the oldest net event, we can assume that the game just started and we must init the state of the game for previous state
        if (oldest_known_state > oldest_net_event)
            game_state[oldest_net_event] = game_state_t(oldest_net_event);
        // calculate the physics. We replay the events from the oldest on the list from server to the latest.
        for (uint64_t tick = oldest_net_event; tick <= game_tick; tick++) {
            random_generator.seed(tick);

            game_state[tick+1] = game_state_t::physics(game_state[tick], events_history[tick]);
        }
        // make sure we don't store too much data in the memory
        clear_history(game_tick - net::events_history_size, events_history);
        clear_history(game_tick - net::events_history_size, game_state);

        // timer
        bool skip_frame = false;
        auto next_time = current_time + microseconds ((long long int)(net::dt*1000000.0));
        if (next_time < std::chrono::steady_clock::now()) {
            skip_frame = true;
        } else {
            std::this_thread::sleep_until(next_time);
            skip_frame = false;
        }
        current_time = next_time;

        if (!skip_frame) {
            graphics.draw(game_state[game_tick+1]);
        }
        for (auto e: events) {
            if (e.type == net::MESSAGE_END_GAME) {
                game_on = false;
                std::cerr << "close game" << std::endl;
            }
        }
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

