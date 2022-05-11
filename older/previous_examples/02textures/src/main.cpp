/**
 * This is the simple hello world for SDL2.
 *
 * physics sample - version offline
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

int main( ) { // int argc, char **argv ) {

	auto window = init_window();
	auto renderer = init_renderer( window );

	// we will be rendering data onto this:
	auto game_texture = load_texture( renderer, "data/face.bmp" );
	auto moving_point_texture = create_texture( renderer, 320, 200 );

	SDL_SetTextureBlendMode( moving_point_texture.get(), SDL_BLENDMODE_BLEND );
	std::pair <int, int>position( 30, 100 );
	std::vector < uint32_t > moving_point_texture_data( 320 * 200 );

	for ( bool game_active = true ; game_active; ) {
		SDL_Event event;
		while ( SDL_PollEvent( &event ) ) {
			if ( event.type == SDL_QUIT ) game_active = false;
		}

		position.first = ( position.first + 1 ) % 320;
		position.second = ( position.second + 2 ) % 200;
		//
		SDL_RenderClear( renderer.get() );

		moving_point_texture_data[position.second * 320 + position.first * 2] = 0x0ffff1111;
		SDL_UpdateTexture( moving_point_texture.get(), NULL, moving_point_texture_data.data(), 320 * sizeof( uint32_t ) );

		SDL_RenderClear( renderer.get() );
		SDL_RenderCopy( renderer.get(), game_texture.get(), NULL, NULL );
		SDL_RenderCopy( renderer.get(), moving_point_texture.get(), NULL, NULL );
		SDL_RenderPresent( renderer.get() );
		SDL_Delay( 100 );
	}
	return 0;
}
