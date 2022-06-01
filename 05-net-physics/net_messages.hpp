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

class player_t {
public:
    int id;
    std::string player_name;

    static player_t from(const std::array<char,128> &data) {
        std::size_t type_code = *((std::size_t *)data.data());
        net::player_t ret;
        ret.id = *((int *)(data.data()+sizeof(std::size_t)));
        for (auto c : std::vector<char> (data.begin()+sizeof(std::size_t)+sizeof(int), data.end())) {
            if (c!=0)ret.player_name = ret.player_name + c;
            else break;
        }
        return ret;
    }

    static std::array<char,128> to_data(const std::any &message) {
        std::array<char,128> data;
        int i = sizeof(size_t);//exclude type
        *(size_t*)data.data() = message.type().hash_code();

        auto player = std::any_cast<net::player_t>( message );
        *((int *)(data.data()+sizeof(std::size_t))) = player.id;
        i+= sizeof(int);
        for (char c : player.player_name)
            data[i++] = c;
        return data;
    }
};

class explosion_t {
public:
    int id;
    uint64_t game_tick;
    double power;
    position_t p;

    static explosion_t from(const std::vector<char> &data) {
        explosion_t explosion;
        return explosion;
    }
    static std::array<char,128> to_data(const std::any &message) {
        std::array<char,128> data;
        int i = sizeof(size_t);//exclude type
        *(size_t*)data.data() = message.type().hash_code();

        auto explosion = std::any_cast<net::explosion_t>( message );

        return data;
    }
};
}


inline std::any deserialize_message(const std::array<char,128> &data) {
    //hash_code
    std::size_t type_code = *((std::size_t *)data.data());
    if (type_code == typeid(net::player_t).hash_code()) {
        return net::player_t::from(data);
    }
    throw std::invalid_argument("bad data type");
}

inline const std::array<char,128> serialize_message(std::any message) {
    std::array<char,128> data;
    for (unsigned int i = 0; i < data.size(); i++) data[i] = 0;
    int i = sizeof(size_t);//exclude type
    *(size_t*)data.data() = message.type().hash_code();
    if (typeid(net::player_t) == message.type()) {
        return net::player_t::to_data(message);
    } else {
        throw std::invalid_argument("bad data type");
    }
    return data;
}


#endif
