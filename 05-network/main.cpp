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

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>


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

    struct addrinfo *addr_p;
    int err = getaddrinfo(addr_txt, port_txt, &hints, &addr_p);
    if (err) {
        return err;
    }
    for (struct addrinfo *rp = addr_p; rp != NULL; rp = rp->ai_next) {
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

void print_ip_addr(struct sockaddr *peer_addr) {
    char s[INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN] = {'\0'};

    switch(peer_addr->sa_family) {
    case AF_INET: {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)peer_addr;
        inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
        break;
    }
    case AF_INET6: {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)peer_addr;
        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), s, INET6_ADDRSTRLEN);
        break;
    }
    default:
        break;
    }
    printf("IP address: %s\n", s);
}

int main_server(int argc, char **argv) {
    int udp_socket = bind_socket(9921);
    if (udp_socket >= 0) {
        printf("(server) Waiting for connection...\n");
        char buffer[128];
        struct sockaddr_storage peer_addr;
        socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
        while (1) {
            ssize_t rsize = recvfrom(udp_socket, buffer, 128, MSG_DONTWAIT,
                                     (struct sockaddr *)&peer_addr, &peer_addr_len);
            if (rsize > 0) {
                printf("(server) Got: '%s'\n", buffer);
                if (rsize > 0) {
                    char host[NI_MAXHOST], service[NI_MAXSERV];
                    getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len, host,
                                NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
                    printf("(server)    from: %s : %s\n", host, service);
                    print_ip_addr((struct sockaddr *)&peer_addr);
                    sendto(udp_socket, buffer, 128, 0, (struct sockaddr *)&peer_addr,
                           peer_addr_len);
                }
               // break;
            }
            printf("waiting...\n");
            sleep(1);
        }
        close(udp_socket);
    }

    return 0;
}

/**
 * @brief The main function.
 *
 * it binds local address
 */

int main_client(int argc, char **argv) {
    int s = bind_socket(9929);
    char ai_addr_buf[500];
    struct sockaddr *ai_addr = (struct sockaddr *)ai_addr_buf;
    socklen_t ai_addrlen = 500;
    char localhost[] = "localhost";
    char *address = (argc > 1) ? argv[1] : localhost;
    if (address[0] == '-') address = localhost;
    printf("addr.. %s\n", address);
    if (find_addresses(address, "9921", -1, ai_addr,
                       &ai_addrlen) == 0) {
        char msg[] = "Hi! How are you?";
        char msg_to_send[129];
        strcpy(msg_to_send, msg);
        if (sendto(s, msg_to_send, 128, 0, ai_addr, ai_addrlen) == -1) {
            fprintf(stderr, "cannot sendto message\n");
            return -1;
        } else {
            struct sockaddr_storage peer_addr;
            socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
            ssize_t rsize = recvfrom(s, msg_to_send, 128, 0,
                                     (struct sockaddr *)&peer_addr, &peer_addr_len);
            printf("(client) got pinged message: '%s'\n", msg_to_send);
        }
        close(s);
    }
    printf("done..\n");
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        if (strcmp(argv[argc-1],"-server") == 0) return main_server(argc, argv);
        if (strcmp(argv[argc-1],"-client") == 0) return main_client(argc, argv);
        printf("wrong argument. Privide -server or -client\n");
    } else {
        printf("-server or -client\n");
    }
    return 0;
}
