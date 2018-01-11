/**
 * @brief the example using network
 *
 ************/

#include "a_game.hpp"

#include "net_server.hpp"
#include "net_client.hpp"

#include <iostream>

using namespace sgd;








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
		std::this_thread::sleep_for(std::chrono::duration<double>(1));
		client.connectTo( config["server"], config["port"], config["name"] );
	} else if ( config.count( "client" ) ) {
		// tylko klient
		std::cout <<"lacze" << std::endl;
		client.connectTo( config["client"], config["port"], config["name"] );
	} else {
		std::cout << "przyklad uruchomienia:" << std::endl;
		std::cout << argv[0] << " server '*' port 1234 name 'Gracz1'" << std::endl;
		std::cout << argv[0] << " client 172.1.2.1 port 1234 name 'Gracz2'" << std::endl;
	}


	// na zajeciach: ogarnijmy komunikacje miedzy serwerem i klientem, synchronizacja i takei ciekawe rzeczy

	Game game;
	game.init(&client, config["name"] );
	game.start();
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
