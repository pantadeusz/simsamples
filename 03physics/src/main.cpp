/**
 * This is the simple hello world for SDL2.
 *
 * The example tries to take care of resource allocation and deallocation
 * using smart pointers.
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
#include <chrono>

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

std::shared_ptr<SDL_Texture> create_texture( const std::shared_ptr<SDL_Renderer> renderer, const int w, const int h ) {
	SDL_Texture * tex = SDL_CreateTexture( renderer.get(),
										   SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h );
	if ( tex == nullptr ) errthrow ( "SDL_CreateTexture" );
	std::shared_ptr<SDL_Texture> texture ( tex, []( SDL_Texture * ptr ) {
		SDL_DestroyTexture( ptr );
	} );
	return texture;
}


using position_t = std::array<double, 2>;
position_t operator+( const position_t &a, const position_t &b ) {
	return {a[0] + b[0], a[1] + b[1]};
}
position_t operator-( const position_t &a, const position_t &b ) {
	return {a[0] - b[0], a[1] - b[1]};
}
position_t operator*( const position_t &a, const double &b ) {
	return {a[0]*b, a[1]*b};
}
position_t operator*( const position_t &a, const position_t &b ) {
	return {a[0]*b[0], a[1]*b[1]};
}
position_t operator/( const position_t &a, const position_t &b ) {
	return {a[0] / b[0], a[1] / b[1]};
}

class Particle {
public:
	position_t position;
	position_t velocity;
	position_t accel;

	Particle( const position_t &p0 ): position( p0 ), velocity{0, 0}, accel{0, 0} {}

	void update( std::chrono::duration<double> &dt ) {
		double dts = dt.count();
		position = position + velocity * dts + accel * dts * dts * 0.5;
		velocity = velocity + accel * dts;
	}

};


int main( ) { // int argc, char **argv ) {

	auto window = init_window( 640, 480 );
	auto renderer = init_renderer( window );

	// we will be rendering data onto this:
	auto game_texture = load_texture( renderer, "data/face.bmp" );
	SDL_SetTextureBlendMode( game_texture.get(), SDL_BLENDMODE_NONE );

	Particle particle( {160, 100} );
	particle.velocity = {10, -50};
	particle.accel = {0, 10};

	std::chrono::duration<double> dt( 0.01 );

	for ( bool game_active = true ; game_active; ) {
		SDL_Event event;
		while ( SDL_PollEvent( &event ) ) {
			if ( event.type == SDL_QUIT ) game_active = false;
		}

		// fizyka

		if ( particle.position[1] > 200 ) {
			if ( particle.velocity[1] > 0 ) particle.velocity[1] = -particle.velocity[1];
		}
		if ( particle.position[0] > 320 ) {
			if ( particle.velocity[0] > 0 ) particle.velocity[0] = -particle.velocity[0];
		}
		if ( particle.position[0] < 0 ) {
			if ( particle.velocity[0] < 0 ) particle.velocity[0] = -particle.velocity[0];
		}

		particle.update( dt );

		// grafika

		SDL_RenderClear( renderer.get() );

		SDL_Rect destRectForFace = {
			.x = (int)(2 * particle.position[0]),
			.y = (int)(2 * particle.position[1]),
			.w = (int)(2 * ( 64 + sqrt( particle.velocity[0] * particle.velocity[0] ) )),
			.h = (int)(2 * ( 64 + sqrt( particle.velocity[1] * particle.velocity[1] ) ))
		};

		SDL_RenderCopy( renderer.get(), game_texture.get(), NULL, &destRectForFace );

		SDL_RenderPresent( renderer.get() );
		SDL_Delay( std::chrono::duration<double, std::milli>( dt ).count() );
	}
	return 0;
}
