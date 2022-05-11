#ifndef __GAMEMAP__
#define __GAMEMAP__

#include <spriteengine.hpp>

class GameMap {
	public:
		std::vector < std::vector < char > > data;

		SDL_Surface * s_sprites;
		SDL_Texture * t_sprites;

		
		GameMap();
		GameMap(const std::string &bmpfname);
		~GameMap();
		
		void draw(double t);
		
		// sprawdza czy punkt jest w kolizji z klockiem
		int getCollision(const std::vector < double > &p);
};

#endif
