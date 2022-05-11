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

#include <spriteengine.hpp>

using namespace std;

// z dokumentacji SDL
Uint32 getpixel(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    switch(bpp) {
    case 1: return *p; break;
    case 2: return *(Uint16 *)p; break;
    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;
    case 4: return *(Uint32 *)p; break;
    default: return 0;
    }
}


tuple < SDL_Surface *, SDL_Texture * > SpriteEngine::getResource(const string &fname) {
	if (cache.count(fname) == 0) {
		SDL_Surface *s = SDL_LoadBMP( fname.c_str() );
		if( s == NULL ) { 
			throw std::runtime_error(SDL_GetError());
		}
		SDL_SetColorKey( s, SDL_TRUE, SDL_MapRGB( s->format, 0, 0xFF, 0xFF ) );
		SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
		cache[fname] = make_tuple(s,t);
	}
	cacheRef[fname] = cacheRef[fname] + 1;
	std::cout << "new " << fname << "\n";
	return cache[fname];
}

void SpriteEngine::freeResource(const string &fname) {
	cacheRef[fname] = cacheRef[fname] - 1;
	if (cacheRef[fname] <= 0) {
		SDL_Surface *s; SDL_Texture *t;
		std::cout << "delete " << fname << "\n";
		std::tie(s, t) = cache[fname];
		SDL_DestroyTexture(t);
		SDL_FreeSurface(s);
		cacheRef.erase(fname);
		cache.erase(fname);
	}
}

void SpriteEngine::startFrame() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 64, 0);
	SDL_RenderClear(renderer);
}

void SpriteEngine::finishFrame() {
	SDL_RenderPresent(renderer);
}

vector < int > SpriteEngine::toScreenCoord(const vector < double > &p) {
	return {(int)((p[0]-cameraPosition[0])*unitSize+screenSize[0]/2),  
		(int)((-p[1]+cameraPosition[1])*unitSize+screenSize[1]/2)};
}

SpriteEngine::SpriteEngine() {
	SDL_Init( SDL_INIT_EVERYTHING ); 
	screenSize = {800,600};
	window = SDL_CreateWindow("Gra na SGD",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenSize[0], screenSize[1], 0);
	renderer = SDL_CreateRenderer(window, -1, 0);
	cameraPosition = {0,0};
	
}

SpriteEngine::~SpriteEngine() {
	SDL_DestroyWindow( window );
	SDL_Quit(); 
}

SpriteEngine spriteEngine;
