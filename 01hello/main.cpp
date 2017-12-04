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

auto errthrow = []( const std::string &e ) {
	SDL_Quit();
	throw std::runtime_error( e + " : " + SDL_GetError() );
};

std::pair< std::shared_ptr<SDL_Window>, std::shared_ptr<SDL_Renderer> > init_renderer() {

	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) errthrow ( "SDL_Init" );

	SDL_Window *win = SDL_CreateWindow( "Witaj w Swiecie", 10, 10, 640, 480, SDL_WINDOW_SHOWN );
	if ( win == nullptr ) errthrow ( "SDL_CreateWindow" );
	std::shared_ptr<SDL_Window> window ( win, []( SDL_Window * ptr ) {
		SDL_DestroyWindow( ptr );
	} );

	SDL_Renderer *ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if ( ren == nullptr ) errthrow ( "SDL_CreateRenderer" );
	std::shared_ptr<SDL_Renderer> renderer ( ren, []( SDL_Renderer * ptr ) {
		SDL_DestroyRenderer( ptr );
	} );

	return {window, renderer};
}

std::shared_ptr<SDL_Texture> load_texture(const std::shared_ptr<SDL_Renderer> renderer, const std::string fname) {
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


int main( ) { // int argc, char **argv ) {
	std::shared_ptr<SDL_Window> window;
	std::shared_ptr<SDL_Renderer> renderer;

	std::tie ( window, renderer ) = init_renderer();
    auto face_texture = load_texture( renderer, "data/face.bmp" );

	for ( int i = 0; i < 60; ++i ) {
		SDL_RenderClear( renderer.get() );
		SDL_RenderCopy( renderer.get(), face_texture.get(), NULL, NULL );
		SDL_RenderPresent( renderer.get() );
		SDL_Delay( 100 );
	}
	return 0;
}
