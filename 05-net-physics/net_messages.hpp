#ifndef ___NET_GAME_ENGINE___
#define ___NET_GAME_ENGINE___

#include <string>
#include <array>
#include <any>
#include <vector>
#include <stdexcept>

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



namespace net {

const int MESSAGE_PLAYER = 1;
const int MESSAGE_EXPLOSION = 2;

struct player_t {
    int type;
    int id;
    char player_name[32];
};


struct explosion_message_t {
    int type;
    int id;
    uint64_t game_tick;
    double power;
    double p[2];
};

union message_t {
    int type;
    player_t player;
    explosion_message_t explosion;
    unsigned char data[128];
};






struct explosion_t {
    int id;
    uint64_t game_tick;
    double power;
    position_t p;

    explosion_message_t to_message() {
        explosion_message_t ret;
        ret.type = MESSAGE_EXPLOSION;
        ret.id = id;
        ret.game_tick = game_tick;
        ret.power = power;
        ret.p[0] = p[0];
        ret.p[1] = p[1];
        return ret;
    }
    static explosion_t from_message(explosion_message_t &message) {
        explosion_t ret;
        ret.id = message.id;
        ret.game_tick = message.game_tick;
        ret.power = message.power;
        ret.p[0] = message.p[0];
        ret.p[1] = message.p[1];
        return ret;
    }
};

}


#endif
