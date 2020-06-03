/**
 * @file server.cpp
 * @author Tadeusz Puźniakowski
 * @brief Simple datagram server
 * @version 0.1
 * @date 2019-01-16
 *
 * @copyright Copyright (c) 2019 Tadeusz Puźniakowski
 * @license MIT
 */

#include "client.h"
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int bind_socket(unsigned int port_nr) {
  if (port_nr < 0)
    port_nr = 9922;
  int s = socket(AF_INET6, SOCK_DGRAM, 0);
  if (s < 0)
    return s;
  const int off = 0;
  if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off))) {
    close(s);
    return -1;
  }

  struct sockaddr_in6 addr;
  bzero(&addr, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port_nr);
  socklen_t alen = sizeof(addr);
  if (bind(s, (struct sockaddr *)(&addr), alen)) {
    close(s);
    return -2;
  }
  return s;
}

class client_info_t {
public:
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len;
  std::string nick;
  int sequence;
};

std::vector<client_info_t> clients;

int main(int argc, char **argv) {
  int sequence = 0;
  int udp_socket = bind_socket(9921);
  if (udp_socket >= 0) {
    printf("(server) Waiting for connection...\n");
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
    while (1) {
      handshake_client_to_server handshake;
      handshake_server_to_client response;

      ssize_t rsize =
          recvfrom(udp_socket, &handshake, sizeof(handshake), MSG_DONTWAIT,
                   (struct sockaddr *)&peer_addr, &peer_addr_len);
      if (rsize > 0) {
        printf("(server) Got: '%s'\n", handshake.nick());
        if (rsize > 0) {
          char host[NI_MAXHOST], service[NI_MAXSERV];
          getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len, host,
                      NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
          printf("(server)    from: %s : %s\n", host, service);

          response.sequence = sequence++;
          response.type = 2;
          strcpy(response.servername(), "Kaczki");
          sendto(udp_socket, &response, sizeof(response), 0,
                 (struct sockaddr *)&peer_addr, peer_addr_len);

          client_info_t newclient;
          newclient.nick = handshake.nick();
          newclient.peer_addr = peer_addr;
          newclient.peer_addr_len = peer_addr_len;
          clients.push_back(newclient);
        }

        if (clients.size() == 2) {
          break;
        }
      }
      printf("waiting...\n");
      sleep(1);
    }

    for (auto &p : clients) {
      gamestart_server_to_client gamestart;
      *gamestart.players() = clients.size();
      sendto(udp_socket, &gamestart, sizeof(gamestart), 0,
             (struct sockaddr *)&p.peer_addr, p.peer_addr_len);
    }

    while (true) {
    }

    close(udp_socket);
  }

  return 0;
}
