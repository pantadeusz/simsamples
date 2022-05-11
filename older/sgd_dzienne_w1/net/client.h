#ifndef __GAME_CLIENT_H___
#define __GAME_CLIENT_H___

#include <string>
#include <vector>

class generic_message_t {
public:
  int sequence;
  int type;
  char data[100];
};

class handshake_client_to_server : public generic_message_t {
public:
  char *nick() {return data;};
  handshake_client_to_server() {
    type = 1;
  }
};

class handshake_server_to_client : public generic_message_t {
public:
  char *servername(){return data;};
  handshake_server_to_client() {
    type = 2;
  }
};

class gamestart_server_to_client : public generic_message_t {
public:
  int *players(){return (int *)data;};
  gamestart_server_to_client() {
    type = 3;
  }
};

class client_t {
  int s; // gniazdo do przesylania danych
  std::vector<char> server_addr_data;
  std::string nick;
  int sequence;

public:
  int start_game() ;
  client_t(const std::string nick_, const std::string host = "localhost",
           const std::string port = "9921");
  ~client_t();
};

#endif
