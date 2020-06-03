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
#include <random>
#include <stdexcept>
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

/**
 * @brief finds server for the given host, port and address family
 *
 * @arg addr_txt address to find
 * @arg port_txt the port to find (as string)
 * @arg ai_family family, if -1, then the family will be AF_UNSPEC
 * @arg ai_addr pointer to memory buffer to store the address
 * @arg ai_addrlen maximal size of addr, it will be replaced with actual size of
 * ai_addr
 *
 * @return 0 on success
 */
int find_addresses(const char *addr_txt, const char *port_txt, int ai_family,
                   struct sockaddr *ai_addr, socklen_t *ai_addrlen) {

  struct addrinfo hints;

  if (ai_family < 0)
    ai_family = AF_UNSPEC;
  bzero((char *)&hints, sizeof(struct addrinfo));
  hints.ai_family = ai_family;    ///< IPv4 or IPv6
  hints.ai_socktype = SOCK_DGRAM; ///< datagram socket

  struct addrinfo *addr_p, *rp;
  int err = getaddrinfo(addr_txt, port_txt, &hints, &addr_p);
  if (err) {
    return err;
  }
  for (rp = addr_p; rp != NULL; rp = rp->ai_next) {
    if (*ai_addrlen < rp->ai_addrlen) {
      fprintf(stderr,
              "[E] The memory buffer for addrlen is too small - %d, but it "
              "needs %d\n",
              *ai_addrlen, rp->ai_addrlen);
    } else {
      memcpy(ai_addr, rp->ai_addr, rp->ai_addrlen); // warning!
      *ai_addrlen = rp->ai_addrlen;
      break;
    }
  }
  freeaddrinfo(addr_p);
  return 0;
}

int client_t::start_game() {
  struct sockaddr *ai_addr = (struct sockaddr *)server_addr_data.data();
  socklen_t ai_addrlen = server_addr_data.size();
  gamestart_server_to_client startmessage;
  ssize_t rsize = recv(s, &startmessage, sizeof(startmessage), 0);
  return *startmessage.players();
}

client_t::client_t(const std::string nick_, const std::string host,
                   const std::string port) {
  nick = nick_;
  sequence = 0;
  std::random_device rd;
  auto gen = std::mt19937(rd());
  auto port_add = std::uniform_int_distribution<>(0, 1000);
  s = bind_socket(9929 + port_add(gen));
  server_addr_data.resize(1024);
  struct sockaddr *ai_addr = (struct sockaddr *)server_addr_data.data();
  socklen_t ai_addrlen = server_addr_data.size();
  if (find_addresses(host.c_str(), port.c_str(), -1, ai_addr, &ai_addrlen) ==
      0) {
    handshake_client_to_server handshake;
    handshake.sequence = sequence++;
    handshake.type = 1;
    strcpy(handshake.nick(), nick_.c_str());

    if (sendto(s, &handshake, sizeof(handshake), 0, ai_addr, ai_addrlen) ==
        -1) {
      fprintf(stderr, "cannot sendto message\n");
      throw std::invalid_argument("cannot find server");
    } else {
      struct sockaddr_storage peer_addr;
      socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
      handshake_server_to_client response;

      ssize_t rsize = recvfrom(s, &response, sizeof(response), 0,
                               (struct sockaddr *)&peer_addr, &peer_addr_len);
      printf("(client) got pinged message: '%s'\n", response.servername());
    }
  } else {
    throw std::invalid_argument("cannot find server");
  }
}
client_t::~client_t() { close(s); }
