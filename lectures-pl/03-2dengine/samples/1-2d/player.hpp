#ifndef __PLAYER__
#define __PLAYER__

#include <gamemap.hpp>

class Player {
	public:
	std::vector < double > position;
	std::vector < double > speed;
	std::vector < double > acceleration;

	SDL_Surface * s_player;
	SDL_Texture * t_player;
	
	Player();
	
	void draw(double t);
	void physics(GameMap &gameMap, double dt);
	bool isOnGround(GameMap &game);
	
};


#endif
