/**
 * This is the simple hello world for SDL2.
 *
 * TODO:
 * 

    1. Load Bitmap

	SDL_Surface *bmp = SDL_LoadBMP( fname.c_str() );
	SDL_FreeSurface( ptr );

    2. Create texture
    
	SDL_Texture *tex = SDL_CreateTextureFromSurface( renderer, bitmap );
	SDL_DestroyTexture( tex );

    3. Load texture using lodepng

  	unsigned error = lodepng::decode(image, width, height, fname);
	if(error) throw std::runtime_error( lodepng_error_text(error));

	SDL_Surface* bitmap = SDL_CreateRGBSurfaceFrom( ... )

	4. draw texture to renderer
	
	SDL_RenderCopy( renderer.get(), game_texture_1.get(), NULL, &dstrect );

    4. collisions

	I will show 8 point collision

 */

#include <SDL2/SDL.h>
#include <lodepng.h>
#include <stdexcept>
#include <memory>
#include <string>
#include <set>
#include <tuple>
#include <iostream>
#include <cstdint>
#include <vector>
#include <random>

	

std::shared_ptr<SDL_Window> init_window( const int width = 320, const int height = 200 ) {
	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) throw std::runtime_error( SDL_GetError() );

	SDL_Window *win = SDL_CreateWindow( "Witaj w Swiecie",
										SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
										width, height, SDL_WINDOW_SHOWN );
	if ( win == nullptr ) throw std::runtime_error( SDL_GetError() );
	std::shared_ptr<SDL_Window> window ( win, []( SDL_Window * ptr ) {
		SDL_DestroyWindow( ptr );
	} );
	return window;
}

std::shared_ptr<SDL_Renderer> init_renderer( std::shared_ptr<SDL_Window> window ) {
	SDL_Renderer *ren = SDL_CreateRenderer( window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if ( ren == nullptr ) throw std::runtime_error( SDL_GetError() );
	std::shared_ptr<SDL_Renderer> renderer ( ren, []( SDL_Renderer * ptr ) {
		SDL_DestroyRenderer( ptr );
	} );
	return renderer;
}

std::shared_ptr<SDL_Texture> load_texture( const std::shared_ptr<SDL_Renderer> renderer, const std::string fname ) {
	SDL_Surface *bmp = SDL_LoadBMP( fname.c_str() );
	if ( bmp == nullptr ) throw std::runtime_error( SDL_GetError() );
	std::shared_ptr<SDL_Surface> bitmap ( bmp, []( SDL_Surface * ptr ) {
		SDL_FreeSurface( ptr );
	} );

	SDL_Texture *tex = SDL_CreateTextureFromSurface( renderer.get(), bitmap.get() );
	if ( tex == nullptr ) throw std::runtime_error( SDL_GetError() );
	std::shared_ptr<SDL_Texture> texture ( tex, []( SDL_Texture * ptr ) {
		SDL_DestroyTexture( ptr );
	} );
	return texture;
}

std::shared_ptr<SDL_Texture> load_png_texture( const std::shared_ptr<SDL_Renderer> renderer, const std::string fname ) {
	std::vector<unsigned char> image;
  	unsigned width, height;
  	unsigned error = lodepng::decode(image, width, height, fname); // load png image to vector (image)
	if(error) throw std::runtime_error( lodepng_error_text(error));

	SDL_Texture * tex = SDL_CreateTexture( renderer.get(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, width, height );
	if ( tex == nullptr ) throw std::runtime_error( SDL_GetError() );
	std::shared_ptr<SDL_Texture> texture ( tex, []( SDL_Texture * ptr ) {SDL_DestroyTexture( ptr );} );
	SDL_SetTextureBlendMode( texture.get(), SDL_BLENDMODE_BLEND ); // obslugujemy polprzezroczystosc
	SDL_UpdateTexture( texture.get(), NULL, image.data(), width * sizeof( uint32_t ) );
	return texture;
}

std::shared_ptr<SDL_Texture> create_texture( const std::shared_ptr<SDL_Renderer> renderer, const int w, const int h ) {
	SDL_Texture * tex = SDL_CreateTexture( renderer.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h );
	if ( tex == nullptr ) throw std::runtime_error( SDL_GetError() );
	std::shared_ptr<SDL_Texture> texture ( tex, []( SDL_Texture * ptr ) {SDL_DestroyTexture( ptr );} );
	return texture;
}

int main( ) { // int argc, char **argv ) {

	auto window = init_window();
	auto renderer = init_renderer( window );

	// we will be rendering data onto this:
	auto game_texture_1 = load_texture( renderer, "data/face.bmp" );
	auto game_texture = load_png_texture( renderer, "data/img.png" );
	auto moving_point_texture = create_texture( renderer, 320, 200 );

	SDL_SetTextureBlendMode( moving_point_texture.get(), SDL_BLENDMODE_BLEND );

	std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(1.0, 10.0);

	for ( bool game_active = true ; game_active; ) {
		SDL_Event event;
		while ( SDL_PollEvent( &event ) ) {
			if ( event.type == SDL_QUIT ) game_active = false;
		}

		SDL_RenderClear( renderer.get() );


		SDL_RenderClear( renderer.get() );
		SDL_Rect dstrect = {0 + dist(mt)-5,0 + dist(mt)-5,320,200};
		SDL_RenderCopy( renderer.get(), game_texture_1.get(), NULL, &dstrect );
		SDL_RenderCopy( renderer.get(), game_texture.get(), NULL, NULL );
		SDL_RenderCopy( renderer.get(), moving_point_texture.get(), NULL, NULL );
		SDL_RenderPresent( renderer.get() );
		SDL_Delay( 10 );
	}
	return 0;
}
