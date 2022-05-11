#ifndef __SPRITEENGINE__
#define __SPRITEENGINE__

#include <SDL.h>
#include <string>
#include <map>
#include <vector>
#include <veclib/veclib.hpp>


const double unitSize = 32;
const std::vector < double > gravity = {0,-10};



// z dokumentacji SDL
Uint32 getpixel(SDL_Surface *surface, int x, int y);


class SpriteEngine {
	protected:
		std::map < std::string, std::tuple < SDL_Surface *, SDL_Texture * > > cache;
		std::map < std::string, int > cacheRef;
	public:

		SDL_Window *window;
		SDL_Renderer *renderer;

		std::vector < double > screenSize;
		std::vector < double > cameraPosition;
		std::vector < double > cameraSpeed;
		
		std::tuple < SDL_Surface *, SDL_Texture * > getResource(const std::string &fname);
		
		void freeResource(const std::string &fname);

		void startFrame();
	
		void finishFrame() ;

		std::vector < int > toScreenCoord(const std::vector < double > &p) ;
		
		SpriteEngine() ;
		
		~SpriteEngine();
};

extern SpriteEngine spriteEngine;


#endif
