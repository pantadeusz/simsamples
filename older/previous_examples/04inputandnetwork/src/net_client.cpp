#include "net_client.hpp"

#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <mutex>

namespace sgd
{

NetTick Client::tick()
{
    NetTick ret;
    recv(sfd, &ret, sizeof(ret), 0);
    return ret;
}

std::map<std::string, Player> Client::recv_players(const std::map<std::string, Player> &players_)
{
    std::map<std::string, Player> players = players_;
    int plrcnt;
    recv(sfd, &plrcnt, sizeof(plrcnt), 0);
    for (int i = 0; i < plrcnt; i++)
    {
        int cnt;
        recv(sfd, &cnt, sizeof(cnt), 0);
        char buff[cnt+1];
        recv(sfd, buff, cnt, 0);
        buff[cnt] = 0;
        Player p;
        if (players_.count(buff)) p = players_.at(buff);
        players[buff] = p;
        std::cout << "Nowy gracz: " << buff << std::endl;
    }
    return players;
}

int Client::handshake()
{
    int retcode = 0;
    size_t name_length = name.length();
    send(sfd, &name_length, 4, MSG_NOSIGNAL);
    if (name_length > 64000) throw "too long name";
    send(sfd, name.c_str(), name.length(), MSG_NOSIGNAL);
    recv(sfd, &retcode, 4, 0);
    std::cout << "RemoteServer::handshake retcode: " << retcode << std::endl;
    return retcode; // ok
}

void Client::connectTo(const std::string remoteAddress, const std::string proto, const std::string player_name_)
{
    int sfd;

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;

    for (unsigned i = 0; i < sizeof(struct addrinfo); i++)
        ((char *)&hints)[i] = 0;

    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Stream socket
    hints.ai_flags = 0;
    hints.ai_protocol = 0; /// Any protocol

    s = getaddrinfo(remoteAddress.c_str(), proto.c_str(), &hints, &result);
    if (s != 0)
    {
        std::stringstream ss;
        ss << "getaddrinfo:: " << gai_strerror(s) << "; " << proto << " : " << remoteAddress << " .";
        throw std::invalid_argument(ss.str());
    }
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
            continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; // Success
        ::close(sfd);
    }

    if (rp == NULL)
    { // No address succeeded
        throw std::invalid_argument("connection error");
    }

    freeaddrinfo(result);
    this->sfd = sfd;
    this->host_s = remoteAddress;
    this->service_s = proto;
    this->name = player_name_;
}

void Client::disconnect()
{
    char c;
    ::shutdown(sfd, SHUT_WR);
    for (int ttl = 20; ttl > 0 && (recv(sfd, &c, 1, MSG_PEEK) > 0); ttl--)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ::close(sfd);
}
}
