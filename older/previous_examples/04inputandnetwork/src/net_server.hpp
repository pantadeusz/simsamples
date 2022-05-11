#ifndef __NET_SERVER__
#define __NET_SERVER__

#include "net_client.hpp"

#include <string>
#include <mutex>
#include <thread>
#include <vector>

namespace sgd {
class RemotePlayer {
public:
	int sfd; // socket descriptor for connected endpoint
	std::string host_s;
	std::string service_s;
    std::string name;
    
    Player player;

    void tick(const NetTick &tick);
    void send_players(const std::vector<RemotePlayer> &players);
	int handshake( const std::vector < RemotePlayer > &players );
	void close();
};

// serwer na gniazdach polaczeniowych
class Server {
protected:
    static void accepting_thread_f(Server*server);
    static void communication_thread_f(Server *server);

public:
	std::mutex clients_m;
	std::vector < RemotePlayer > clients;
	int listening_socket;
    std::thread acceptingThread;
    bool serverWorking;
    std::thread communicationThread;
	RemotePlayer acceptConnection();
	void start( std::string host, std::string port );
    void stop();
    Server() = default;
    Server(const Server& that) = delete;

};

}


#endif // !__NET_SERVER__#define 
