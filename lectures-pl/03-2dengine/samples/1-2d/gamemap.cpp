#include <SDL.h>
#include <vector>
#include <tuple>
#include <map>
#include <cmath>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>
#include <stdexcept>

#include <gamemap.hpp>

using namespace std;

void GameMap::draw(double t) {
	SDL_Rect srcrect;
	SDL_Rect dstrect;
	for (int y = 0; y < data.size(); y++) 
		for (int x = 0; x < data[y].size(); x++) {
			int s = (data[y][x] >> 4) & 0x0f;
			if (s != 0) {
				srcrect = {(s>>2)*32, (s & 3)*32, 32,32};
				auto pp = spriteEngine.toScreenCoord({(double)x,(double)y});
				dstrect = {
					pp[0], pp[1],
					(int)unitSize,(int)unitSize};
				SDL_RenderCopy(spriteEngine.renderer, t_sprites, &srcrect, &dstrect);
			}
		}
}

GameMap::GameMap() {
	std::tie(s_sprites, t_sprites) = spriteEngine.getResource("img/sprites.bmp"); 
}
		
GameMap::~GameMap() {
	spriteEngine.freeResource("img/sprites.bmp");
}

GameMap::GameMap(const string &bmpfname) {
	std::tie(s_sprites, t_sprites) = spriteEngine.getResource("img/sprites.bmp"); 
	SDL_Surface * s_map = SDL_LoadBMP( bmpfname.c_str() );
	if( s_map == NULL ) { 
		throw std::runtime_error(SDL_GetError());
	}
	for (int y = s_map->h-1; y >= 0 ; y--) {
		vector < char > row;
		for (int x = 0; x < s_map->w; x++) {
			row.push_back(getpixel(s_map, x,y));
		}
		data.push_back(row);
	}
	SDL_FreeSurface(s_map);
}

int GameMap::getCollision(const vector < double > &p) {
	if ((p[0] < data[0].size()) &&
		(p[0] >= 0) && 
		(p[1] < data.size()) &&
		(p[1] >= 0)) {
			return data[(int)p[1]][(int)p[0]] >> 4;
	} else {
		return 1; // kolizja
	}
}
