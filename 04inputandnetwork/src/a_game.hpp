#ifndef __A_GAME__
#define  __A_GAME__

#include "net_client.hpp"
#include "p_particles.hpp"
#include "vid_sdl.hpp"
#include "net_client.hpp"
#include <map>

namespace sgd {


class Game {
protected:

// std::chrono::high_resolution_clock::now()
    //std::chrono::time_point<std::chrono::high_resolution_clock,duration_t> prevTime;
	//std::chrono::steady_clock::time_point prevTime;
	std::chrono::high_resolution_clock::time_point prevTime;


	std::shared_ptr<SDL_Window> window;
	std::shared_ptr<SDL_Renderer> renderer;
	std::shared_ptr<SDL_Texture> particle_tex;

	std::shared_ptr<SDL_Texture> player_tex;

	std::function < void ( const Particle & ) > draw_ball_particle_f;


	std::map < std::string, Player > players;

	std::string player_name;

	Client *clientConnection;

public:
	// przyrost czasu symulacji w sekundach
	duration_t dt;

	// efekty cząsteczkowe
	std::vector < Particle > particles;

	position_t scaleFactor;

	// initialize video card
	void init_video();

	// loading textures
	void init_textures();


	// creating data and functions for particle effects
	void init_particles();

	// prepare everything to game
	void init( Client *client, const std::string &player_name_ = "player" );

	// start game. This initializes timer!
	void start();

	// get inputs from users
	int l_input();

	// apply physics to everything
	void l_physics();
	// draw whole scene
	void l_draw();
	// wait for next frame
	void l_sync();

};

}


#endif
