#include "net_server.hpp"

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

void RemotePlayer::tick(const NetTick &tick)
{
    send(sfd, &tick, sizeof(tick), MSG_NOSIGNAL);
}

void RemotePlayer::send_players(const std::vector<RemotePlayer> &players)
{
    int plrcnt = players.size();
    send(sfd, &plrcnt, sizeof(plrcnt), MSG_NOSIGNAL);
    for (const auto &player : players)
    {
        int cnt = player.name.size();
        send(sfd, &cnt, sizeof(cnt), MSG_NOSIGNAL);
        send(sfd, player.name.c_str(), cnt, MSG_NOSIGNAL);
    }
}

int RemotePlayer::handshake(const std::vector<RemotePlayer> &players)
{
    std::cout << "RemotePlayer::handshake: nowy gracz: " << std::endl;
    int retcode = 0;
    size_t name_length = 0;
    recv(sfd, &name_length, 4, 0);
    std::vector<char> name_v(name_length);
    recv(sfd, name_v.data(), name_length, 0);
    name = std::string(name_v.begin(), name_v.end());
    std::cout << "RemotePlayer::handshake: nowy gracz: " << name << std::endl;
    retcode = 0; // ok
    for (const auto &op : players)
    {
        if (op.name == name)
        {
            retcode = -1;
        }
    }
    send(sfd, &retcode, 4, MSG_NOSIGNAL);
    return retcode;
}

void RemotePlayer::close()
{
    char c;
    ::shutdown(sfd, SHUT_WR);
    for (int ttl = 20; ttl > 0 && (recv(sfd, &c, 1, MSG_PEEK) > 0); ttl--)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ::close(sfd);
}

// serwer na gniazdach polaczeniowych
RemotePlayer Server::acceptConnection()
{
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len = sizeof(struct sockaddr_storage);

    std::string host_s;
    std::string service_s;
    int csfd; // connected socket..

    if ((csfd = ::accept(listening_socket, (struct sockaddr *)&peer_addr, &peer_addr_len)) == -1)
    {
        throw std::invalid_argument("could not accept connection!");
    }
    else
    {
        char host[NI_MAXHOST], service[NI_MAXSERV];
        getnameinfo((struct sockaddr *)&peer_addr,
                    peer_addr_len, host, NI_MAXHOST,
                    service, NI_MAXSERV, NI_NUMERICSERV);
        host_s = host;
        service_s = service;
    }
    RemotePlayer ret;
    ret.sfd = csfd;
    ret.host_s = host_s;
    ret.service_s = service_s;
    std::cout << "Podlaczyl sie gracz " << host_s << " " << std::endl;
    return ret;
}

void Server::accepting_thread_f(Server *server)
{
    try
    {
        while (true)
        {
            RemotePlayer rp = server->acceptConnection();
            std::lock_guard<std::mutex> lock(server->clients_m);
            if (rp.handshake(server->clients) == 0)
            {
                server->clients.push_back(rp);
            }
            else
            {
                std::cout << "acceptingThread:: dropping duplicate client " << std::endl;
                rp.close();
            }
        }
    }
    catch (...)
    {
        std::cout << "ending listening thread.." << std::endl;
    }
}

void Server::communication_thread_f(Server *server)
{
    double dt = 0.033;
    unsigned client_count = 0;
    auto prevTime = std::chrono::high_resolution_clock::now();
    while (server->serverWorking)
    {
        // czekamy
        std::this_thread::sleep_until(prevTime + std::chrono::duration<double>(dt));
        prevTime = std::chrono::high_resolution_clock::now(); //prevTime + std::chrono::duration<double>(dt);
        {
            // wysylamy
            std::lock_guard<std::mutex> lock(server->clients_m);
            for (auto &client : server->clients)
            {
                if (client_count != server->clients.size())
                {
                    NetTick tick = {1, dt};
                    client.tick(tick);
                    client.send_players(server->clients);
                }
                else
                {
                    NetTick tick = {0, dt};
                    client.tick(tick);
                }
            }
            client_count = server->clients.size();
        }
    }
}

void Server::start(std::string host, std::string port)
{
    int yes = 1;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;

    for (unsigned int i = 0; i < sizeof(struct addrinfo); i++)
        ((char *)&hints)[i] = 0;
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
    if (s != 0)
    {
        std::stringstream ss;
        ss << "ListeningSocket getaddrinfo:: " << gai_strerror(s) << "; port= " << port << std::endl;
        throw std::invalid_argument(ss.str());
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        listening_socket = socket(rp->ai_family, rp->ai_socktype,
                                  rp->ai_protocol);
        if (listening_socket == -1)
            continue;

        if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        {
            throw std::invalid_argument("error at: setsockopt( listening_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof( int ) )");
        }

        if (bind(listening_socket, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            break;
        }
        ::close(listening_socket);
    }

    if (rp == NULL)
    {
        std::stringstream ss;
        ss << "ListeningSocket unable to bind address:: " << gai_strerror(s) << "; " << port;
        throw std::invalid_argument(ss.str());
    }
    freeaddrinfo(result);
    if (listen(listening_socket, 32) == -1)
    {
        throw std::invalid_argument("listen error");
    }

    acceptingThread = std::thread(accepting_thread_f, this);
    acceptingThread.detach();
    serverWorking = true;
    communicationThread = std::thread(communication_thread_f, this);
    communicationThread.detach();
}

void Server::stop()
{
    std::lock_guard<std::mutex> lock(clients_m);
    for (auto &client : clients)
    {
        std::cout << "closing client " << std::endl;
        client.close();
    }
    serverWorking = false;
    ::close(listening_socket);
}
}
