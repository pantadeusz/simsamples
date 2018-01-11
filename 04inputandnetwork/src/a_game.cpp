#include "a_game.hpp"

//#include "p_particles.hpp"
//#include "vid_sdl.hpp"
//
#include <thread>
//#include <vector>
//#include <algorithm>
//#include <random>
//#include <map>
//#include <list>
//#include <functional>

#include <iostream>

using namespace std::chrono_literals;

namespace sgd
{

// initialize video card
void Game::init_video()
{
	window = init_window(640, 480);
	renderer = init_renderer(window);
}

// loading textures
void Game::init_textures()
{
	particle_tex = generate_texture_ball(renderer, 3);
	player_tex = generate_texture_ball(renderer, 8, 0x0ff0000);
}

// creating data and functions for particle effects
void Game::init_particles()
{
	draw_ball_particle_f = [&](const Particle &p) {
		auto pos = p.position * scaleFactor;
		SDL_Rect destRectForFace = {.x = (int)(pos[0] - 3), .y = (int)(pos[1] - 3), .w = 7, .h = 7};
		SDL_RenderCopyEx(renderer.get(), particle_tex.get(), NULL, &destRectForFace, destRectForFace.x, NULL, SDL_FLIP_NONE);
	};
}

// prepare everything to game
void Game::init(Client *client, const std::string &player_name_)
{

	clientConnection = client;

	// introduce self!!
	if (clientConnection->handshake() < 0)
	{
		clientConnection->disconnect();
		throw std::invalid_argument("connection refused - duplicate name!");
	}

	player_name = player_name_;
	particles.clear();
	scaleFactor = {10, 10};
	dt = 0.015s;
	players.clear();
	players[player_name].angle = 0;
	players[player_name].reload_time = 0;
	players[player_name].init({10, 10}, {0, 0}, {0, 0});
	players[player_name].on_update = [&](Particle &self, const duration_t &dt) {
		self.ttl = 100;
		auto ret = Particle::get_default_update_f()(self, dt);
		//self.accel = self.accel * 0.8; // oszukany opor
		for (int i = 0; i < 2; i++)
		{
			if (self.position[i] > 40)
			{
				self.position[i] = 40;
				self.velocity[i] *= -0.8;
			}
			if (self.position[i] < 0)
			{
				self.position[i] = 0;
				self.velocity[i] *= -0.8;
			}
		}
		return ret;
	};
	init_video();
	init_textures();
	init_particles();
}

// start game. This initializes timer!
void Game::start()
{
	prevTime = std::chrono::high_resolution_clock::now(); //std::chrono::steady_clock::now(); // w sekundach
}

// get inputs from users
int Game::l_input()
{
	int closeGame = false;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			closeGame = true;
			break;
		}
	}

	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_SPACE])
	{
		players[player_name].action_to_do.fire = 1;
	}
	else
	{
		players[player_name].action_to_do.fire = 0;
	}
	players[player_name].action_to_do.rotate = 0;
	if (state[SDL_SCANCODE_LEFT])
	{
		players[player_name].action_to_do.rotate = -100;
	}
	if (state[SDL_SCANCODE_RIGHT])
	{
		players[player_name].action_to_do.rotate = 100;
	}
	if (state[SDL_SCANCODE_UP])
	{
		players[player_name].action_to_do.move = 1;
	}
	else
	{
		players[player_name].action_to_do.move = 0;
	}

	// non local players!!!
	return closeGame;
}

// apply physics to everything
void Game::l_physics()
{
	// fizyka
	for (auto &p : players)
	{
		// obsluga akcji gracza
		auto &player = p.second;
		if (player.action_to_do.rotate != 0)
		{
			player.angle = player.angle + player.action_to_do.rotate * 0.1 * dt.count();
		}

		if (player.action_to_do.move > 0)
		{
			player.accel = {50.0 * ::cos(player.angle), 50.0 * ::sin(player.angle)};
		}
		else
		{
			player.accel = {0.0, 0.0};
		}

		// czy dana postac strzela?
		if (player.action_to_do.fire > 0)
		{
			if (player.reload_time <= 0)
			{
				position_t np = {player.position[0], player.position[1]};
				position_t v = {::cos(player.angle), ::sin(player.angle)};
				np = np + v * 1.0;
				v = v * 20 + player.velocity;
				Particle particle;
				particle.init(np, v, {0, 0});
				particle.ttl = 20;
				particle.draw_particle = draw_ball_particle_f;
				particle.on_update = [this](Particle &self, const duration_t &dt) {
					//self.ttl = ;
					auto ret = Particle::get_default_update_f()(self, dt);
					for (auto &p : players)
					{
						auto distv = p.second.position - self.position;
						double l = ::sqrt(distv[0] * distv[0] + distv[1] * distv[1]);
						if (l < 1)
						{
							self.ttl = -1;
							//std::cout << "trafiono gracza " << p.first  << std::endl;
						}
					}
					return ret;
				};
				particles.push_back(particle);
				player.reload_time = 0.1;
			}
		}
		if (player.reload_time > 0)
			player.reload_time -= dt.count();

		// fizyka gracza - korzystamy z tego ze jest to tez czasteczka
		player.on_update(player, dt);
	}
	particles = calculate_particles(particles, dt);
}

// draw whole scene
void Game::l_draw()
{
	// grafika
	SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 0);
	SDL_RenderClear(renderer.get());

	draw_particles(particles);

	for (auto &p : players)
	{
		auto &player = p.second;
		auto pos = player.position * scaleFactor;
		SDL_Rect destRectForFace = {.x = (int)(pos[0] - 5), .y = (int)(pos[1] - 5), .w = 11, .h = 11};
		SDL_RenderCopyEx(renderer.get(), particle_tex.get(), NULL, &destRectForFace, player.angle * 57.2957795, NULL, SDL_FLIP_NONE);
	}

	SDL_RenderPresent(renderer.get());
}

// wait for next frame
void Game::l_sync()
{
	//std::cout << "czekamy na serwer.." << std::endl;
	auto tick = clientConnection->tick();
	////std::cout << "nowa klatka... " << std::endl;
	if (tick.flags == 1)
	{
		std::cout << "Zmiana liczby graczy!!" << std::endl;
		players = clientConnection->recv_players(players);
		auto &ourPlayer = players[player_name];
		for (auto &p : players)
		{
			if (&p.second != &ourPlayer) p.second = ourPlayer;
		}
	}
	//std::this_thread::sleep_until(prevTime + std::chrono::duration<double>(dt));
	//prevTime = std::chrono::high_resolution_clock::now(); //prevTime + std::chrono::duration<double>(dt);
}
}
