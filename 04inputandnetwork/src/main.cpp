/**
 * @brief the example using network
 *
 ************/

#include "a_game.hpp"

#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <mutex>


using namespace sgd;




class RemotePlayer {
public:
	int sfd; // socket descriptor for connected endpoint
	std::string host_s;
	std::string service_s;


	std::string name;

	int handshake( std::vector < RemotePlayer > players ) {
		int retcode = 0;
		size_t name_length;
		recv( sfd, &name_length, 4, 0 );
		std::vector < char > name_v( name_length );
		recv( sfd, name_v.data(), name_length, 0 );
		name = std::string( name_v.begin(), name_v.end() );
		std::cout << "RemotePlayer::handshake: nowy gracz: " << name << std::endl;
		retcode = 0; // ok
		for (const auto & op : players) {
			if (op.name == name) {
				retcode = -1;
			}
		}
		send( sfd, &retcode, 4, MSG_NOSIGNAL );
		return retcode;
	}

	void close() {
		char c;
		::shutdown ( sfd, SHUT_WR );
		for ( int ttl = 20; ttl > 0 && ( recv( sfd, &c, 1, MSG_PEEK ) > 0 ); ttl-- ) {
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}
		::close ( sfd );
	}
};

// serwer na gniazdach polaczeniowych
class Server {

public:
	std::recursive_mutex clients_m;
	std::vector < RemotePlayer > clients;

	int listening_socket;

	std::thread acceptingThread;

	RemotePlayer acceptConnection() {
		struct sockaddr_storage peer_addr;
		socklen_t peer_addr_len = sizeof( struct sockaddr_storage );

		std::string host_s;
		std::string service_s;
		int csfd; // connected socket..

		if ( ( csfd = ::accept( listening_socket, ( struct sockaddr * )&peer_addr, &peer_addr_len ) ) == -1 ) {
			throw std::invalid_argument( "could not accept connection!" );
		} else {
			char host[NI_MAXHOST], service[NI_MAXSERV];
			getnameinfo( ( struct sockaddr * ) &peer_addr,
						 peer_addr_len, host, NI_MAXHOST,
						 service, NI_MAXSERV, NI_NUMERICSERV );
			host_s	= host;
			service_s = service;
		}
		RemotePlayer ret;
		ret.sfd = csfd;
		ret.host_s = host_s;
		ret.service_s = service_s;
		std::cout << "Podlaczyl sie gracz " << host_s << " " << service_s << " " << std::endl;
		return ret;
	}


	void start( std::string host, std::string port ) {
		int yes = 1;
		struct addrinfo hints;
		struct addrinfo *result, *rp;
		int s;

		for ( unsigned int i = 0; i < sizeof( struct addrinfo ); i++ ) ( ( char * )&hints )[i] = 0;
		hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
		hints.ai_socktype = SOCK_STREAM; /* Stream socket */
		hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
		hints.ai_protocol = 0;          /* Any protocol */
		hints.ai_canonname = NULL;
		hints.ai_addr = NULL;
		hints.ai_next = NULL;

		s = getaddrinfo( host.c_str(), port.c_str(), &hints, &result );
		if ( s != 0 ) {
			std::stringstream ss;
			ss << "ListeningSocket getaddrinfo:: " << gai_strerror( s ) << "; port= " << port << std::endl;
			throw std::invalid_argument( ss.str() );
		}

		for ( rp = result; rp != NULL; rp = rp->ai_next ) {
			listening_socket = socket( rp->ai_family, rp->ai_socktype,
									   rp->ai_protocol );
			if ( listening_socket == -1 ) continue;

			if ( setsockopt( listening_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( yes ) ) == -1 ) {
				throw std::invalid_argument( "error at: setsockopt( listening_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof( int ) )" );
			}

			if ( bind( listening_socket, rp->ai_addr, rp->ai_addrlen ) == 0 ) {
				break;
			}
			::close( listening_socket );
		}

		if ( rp == NULL ) {
			std::stringstream ss;
			ss << "ListeningSocket unable to bind address:: " << gai_strerror( s ) << "; " << port;
			throw std::invalid_argument( ss.str() );
		}
		freeaddrinfo( result );
		if ( listen( listening_socket, 32 ) == -1 ) {
			throw std::invalid_argument( "listen error" );
		}

		acceptingThread = std::thread ( [&]() {
			try {
				while ( true ) {
					RemotePlayer rp = acceptConnection();
					std::lock_guard<std::recursive_mutex> lock( clients_m );
					if (rp.handshake( clients ) == 0) clients.push_back( rp );
					else {
						std::cout << "acceptingThread:: dropping duplicate client " << std::endl;
						rp.close();
					}
				}
			} catch ( ... ) {
				std::cout << "ending listening thread.." << std::endl;
			}
		} );
		acceptingThread.detach();
	}

	void stop() {
		std::lock_guard<std::recursive_mutex> lock( clients_m );
		for ( auto & client : clients ) {
			std::cout << "closing client " << std::endl;
			client.close();
		}
		::close( listening_socket );
	}
};


class Client {
public:
int sfd; // socket descriptor for connected endpoint
	std::string host_s;
	std::string service_s;
	std::string name;

	int handshake() {
		int retcode = 0;
		size_t name_length = name.length();
		send( sfd, &name_length, 4, MSG_NOSIGNAL );
		send( sfd, name.c_str(), name.length(), MSG_NOSIGNAL );
		recv( sfd, &retcode, 4, 0 );
		std::cout << "RemoteServer::handshake retcode: " << retcode << std::endl;
		return retcode; // ok
	}

	void connectTo( const std::string remoteAddress, const std::string proto, const std::string player_name_ ) {
		int sfd;

		struct addrinfo hints;
		struct addrinfo *result, *rp;
		int s;

		for ( unsigned i = 0; i < sizeof( struct addrinfo ); i++ ) ( ( char * )&hints )[i] = 0;

		hints.ai_family = AF_UNSPEC;    // Allow IPv4 or IPv6
		hints.ai_socktype = SOCK_STREAM; // Stream socket
		hints.ai_flags = 0;
		hints.ai_protocol = 0;          /// Any protocol

		s = getaddrinfo( remoteAddress.c_str(), proto.c_str(), &hints, &result );
		if ( s != 0 ) {
			std::stringstream ss;
			ss << "getaddrinfo:: " << gai_strerror( s ) << "; " << proto << " : " << remoteAddress << " .";
			throw std::invalid_argument( ss.str() );
		}
		for ( rp = result; rp != NULL; rp = rp->ai_next ) {
			sfd = socket( rp->ai_family, rp->ai_socktype,
						  rp->ai_protocol );
			if ( sfd == -1 )
				continue;
			if ( connect( sfd, rp->ai_addr, rp->ai_addrlen ) != -1 )
				break;                  // Success
			::close( sfd );
		}

		if ( rp == NULL ) {             // No address succeeded
			throw std::invalid_argument( "connection error" );
		}

		freeaddrinfo( result );
		this->sfd = sfd;
		this->host_s = remoteAddress;
		this->service_s = proto;
		this->name = player_name_;

		// introduce self!!
		if ( this->handshake() < 0 ) {
			this->disconnect();
			throw std::invalid_argument( "connection refused - duplicate name!" );
		}
	}

	void disconnect() {
		char c;
		::shutdown ( sfd, SHUT_WR );
		for ( int ttl = 20; ttl > 0 && ( recv( sfd, &c, 1, MSG_PEEK ) > 0 ); ttl-- ) {
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}
		::close ( sfd );
	}

};


int main( int argc, char **argv ) {

	// przyklad uruchomienia:
	// ./sdldemoapp server '*' port 1234 name 'Gracz1'
	// ./sdldemoapp client 172.1.2.1 port 1234 name 'Gracz2'

	std::map < std::string, std::string > config;
	config["port"] = "12345";
	config["name"] = "Player";
	for ( int i = 1; i < argc - 1; i += 2 ) {
		config[argv[i]] = argv[i + 1];
	}
	Server server;
	Client client;

	if ( config.count( "server" ) ) {
		// uruchamiamy serwer
		server.start( config["server"], config["port"] );
		client.connectTo( config["server"], config["port"], config["name"] );
	} else if ( config.count( "client" ) ) {
		// tylko klient
		client.connectTo( config["client"], config["port"], config["name"] );
	} else {
		std::cout << "przyklad uruchomienia:" << std::endl;
		std::cout << argv[0] << " server '*' port 1234 name 'Gracz1'" << std::endl;
		std::cout << argv[0] << " client 172.1.2.1 port 1234 name 'Gracz2'" << std::endl;
	}


	// na zajeciach: ogarnijmy komunikacje miedzy serwerem i klientem, synchronizacja i takei ciekawe rzeczy

	Game game;
	game.init();
	for ( bool game_active = true ; game_active; ) {
		if ( game.l_input() ) game_active = false;
		game.l_physics();
		game.l_draw();
		game.l_sync();
	}


	if ( config.count( "server" ) ) {
		std::thread t( [&]() {
			client.disconnect();
		} );
		server.stop();
		t.join();
	} else if ( config.count( "client" ) ) {
		client.disconnect();
	}

	return 0;
}
