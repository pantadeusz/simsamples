#ifndef __NET_CLIENT__
#define __NET_CLIENT__

#include "p_particles.hpp"
#include <string>
#include <map>

namespace sgd {

struct NetTick {
    int flags;
    double dt;
};

struct PlayerAction {
	int move;
	int rotate;
	int fire;
};

class Player : public Particle {
public:
	double angle;
	double reload_time;

	PlayerAction action_to_do;
};

class Client
{
  public:
    int sfd; // socket descriptor for connected endpoint
    std::string host_s;
    std::string service_s;
    std::string name;

    // oczekiwanie na nowa klatke
    std::map<std::string, Player> recv_players(const std::map<std::string, Player> &players_);
    NetTick tick();

    int handshake();
    void connectTo(const std::string remoteAddress,
                   const std::string proto,
                   const std::string player_name_);
    void disconnect();
};
}

#endif // !__NET_CLIENT__#define
