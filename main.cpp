
/**
 * This is the simple hello world for SDL2.
 */

#include <SDL2/SDL.h>
#include <stdexcept>

int main( int argc, char **argv ) {
	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
		throw std::runtime_error( std::string("SDL_Init: ") + SDL_GetError() );
		return 1;
	}
	return 0;
}
