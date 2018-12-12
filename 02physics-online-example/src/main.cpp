/**
 * This is the simple hello world for SDL2.
 *
 * Simple physics animation
 */

#include <SDL2/SDL.h>
#include <stdexcept>
#include <memory>
#include <string>
#include <set>
#include <tuple>
#include <iostream>
#include <cstdint>
#include <vector>
#include <array>


auto errthrow = []( const std::string &e ) {
	std::string errstr = e + " : " + SDL_GetError();
	SDL_Quit();
	throw std::runtime_error( errstr );
};

std::shared_ptr<SDL_Window> init_window( const int width = 320, const int height = 200 ) {
	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) errthrow ( "SDL_Init" );

	SDL_Window *win = SDL_CreateWindow( "Witaj w Swiecie",
										SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
										width, height, SDL_WINDOW_SHOWN );
	if ( win == nullptr ) errthrow ( "SDL_CreateWindow" );
	std::shared_ptr<SDL_Window> window ( win, []( SDL_Window * ptr ) {
		SDL_DestroyWindow( ptr );
	} );
	return window;
}

std::shared_ptr<SDL_Renderer> init_renderer( std::shared_ptr<SDL_Window> window ) {
	SDL_Renderer *ren = SDL_CreateRenderer( window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if ( ren == nullptr ) errthrow ( "SDL_CreateRenderer" );
	std::shared_ptr<SDL_Renderer> renderer ( ren, []( SDL_Renderer * ptr ) {
		SDL_DestroyRenderer( ptr );
	} );
	return renderer;
}

std::shared_ptr<SDL_Texture> load_texture( const std::shared_ptr<SDL_Renderer> renderer, const std::string fname ) {
	SDL_Surface *bmp = SDL_LoadBMP( fname.c_str() );
	if ( bmp == nullptr ) errthrow ( "SDL_LoadBMP" );
	std::shared_ptr<SDL_Surface> bitmap ( bmp, []( SDL_Surface * ptr ) {
		SDL_FreeSurface( ptr );
	} );

	SDL_Texture *tex = SDL_CreateTextureFromSurface( renderer.get(), bitmap.get() );
	if ( tex == nullptr ) errthrow ( "SDL_CreateTextureFromSurface" );
	std::shared_ptr<SDL_Texture> texture ( tex, []( SDL_Texture * ptr ) {
		SDL_DestroyTexture( ptr );
	} );
	return texture;
}

using pos_t = std::array<double, 2> ;

pos_t operator +( const pos_t &a, const pos_t &b ) {
	return {a[0] + b[0], a[1] + b[1]};
}
pos_t operator -( const pos_t &a, const pos_t &b ) {
	return {a[0] - b[0], a[1] - b[1]};
}
pos_t operator *( const pos_t &a, const pos_t &b ) {
	return {a[0]*b[0], a[1]*b[1]};
}
pos_t operator *( const pos_t &a, const double &b ) {
	return {a[0]*b, a[1]*b};
}


int main( ) { // int argc, char **argv ) {
	auto window = init_window( 640, 480 );
	auto renderer = init_renderer( window );

	auto cannon_texture = load_texture( renderer, "armata.bmp" );
	auto bullet_texture = load_texture( renderer, "kula.bmp" );


	pos_t gun_p  = {1, 10};
	pos_t bullet_p = {2, 10};
	pos_t bullet_v = {20, 0};
	pos_t bullet_a = {0, 4};
	double scale = 5;
	double dt = 1 / 30.0;

	for ( bool game_active = true ; game_active; ) {
		SDL_Event event;
		while ( SDL_PollEvent( &event ) ) {
			if ( event.type == SDL_QUIT ) {game_active = false;}
			if ( event.type == SDL_KEYDOWN ) {
				if ( event.key.keysym.sym == SDLK_SPACE ) {
					gun_p  = {1, 10};
					bullet_p = {2, 10};
					bullet_v = {20, 0};
					bullet_a = {0, 4};
				}
			}
		}



		// fizyka
		bullet_p = bullet_p + bullet_v * dt + bullet_v * bullet_a * 0.5 * dt * dt;
		bullet_v = bullet_v + bullet_a * dt;
		bullet_a = bullet_a - bullet_v * 0.001;
		// grafika
		SDL_RenderClear( renderer.get() );

		SDL_Rect dstrect = {.x = ( int )( gun_p[0] * scale ), .y = ( int )( gun_p[1] * scale ), .w = 64, .h = 64};
		SDL_Point center = {.x = 32, .y = 32};
		SDL_RenderCopyEx( renderer.get(), cannon_texture.get(), NULL,  &dstrect, 5, &center, SDL_FLIP_NONE );

		dstrect = {.x = ( int )( bullet_p[0] * scale ), .y = ( int )( bullet_p[1] * scale ), .w = 64, .h = 64};
		center = {.x = 32, .y = 32};
		SDL_RenderCopyEx( renderer.get(), bullet_texture.get(), NULL,  &dstrect, 0, &center, SDL_FLIP_NONE );

		//		SDL_RenderDrawPoint(renderer.get(), 100, 100); // https://wiki.libsdl.org/SDL_RenderDrawPoint

		SDL_RenderPresent( renderer.get() );
		SDL_Delay( ( int )( dt / 1000 ) );
	}
	return 0;
}
