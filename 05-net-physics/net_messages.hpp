#ifndef ___NET_GAME_ENGINE___
#define ___NET_GAME_ENGINE___

#include <string>
#include <array>
#include <any>
#include <vector>
#include <stdexcept>

struct position_t {
    double y,x;
    double &operator[](std::size_t i) {
        return (i&1)?x:y;
    }
    double operator[](std::size_t i) const {
        return (i&1)?x:y;
    }
};

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



namespace net {

const double fps = 60.0;
const double dt = 1.0/fps;
const int server_dt_multiply = 4;
const double server_dt = dt*server_dt_multiply;

const uint64_t events_history_size = 100;

const int MESSAGE_PLAYER = 1;
const int MESSAGE_EXPLOSION = 2;
const int MESSAGE_END_GAME = 3;
const int MESSAGE_TICK = 4;


union message_t;

struct player_t {
    int type;
    uint64_t game_tick;
    int id;
    char player_name[32];
    message_t m() const;
};

struct end_game_t {
    int type;
    uint64_t game_tick;
    bool quit;
    message_t m() const;
};

struct explosion_t {
    int type;
    uint64_t game_tick;
    int id;
    double power;
    position_t p;
    message_t m() const;
};
struct tick_t {
    int type;
    uint64_t game_tick;
    message_t m() const;
};

union message_t {
    int type;
    player_t player;
    explosion_t explosion;
    end_game_t end_game;
    tick_t tick;
    unsigned char data[128];
    inline std::any v() const;
};

inline message_t player_t::m() const {
    message_t ret;
    ret.player = *this;
    ret.type = MESSAGE_PLAYER;
    return ret;
}
inline message_t explosion_t::m() const {
    message_t ret;
    ret.explosion = *this;
    ret.type = MESSAGE_EXPLOSION;
    return ret;
}
inline message_t end_game_t::m() const {
    message_t ret;
    ret.end_game = *this;
    ret.type = MESSAGE_END_GAME;
    return ret;
}

inline message_t tick_t::m() const {
    message_t ret;
    ret.tick = *this;
    ret.type = MESSAGE_TICK;
    return ret;
}

inline std::any message_t::v() const {
    switch (type)
    {
    case MESSAGE_END_GAME:
        return end_game;
    case MESSAGE_EXPLOSION:
        return explosion;
    case MESSAGE_PLAYER:
        return player;
    case MESSAGE_TICK:
        return tick;
    default:
        throw std::invalid_argument("bad message type");
    }
}

}


#endif
