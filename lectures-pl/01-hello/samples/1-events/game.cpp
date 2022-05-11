#include <SDL.h>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>




int main( int argc, char* args[] ) { 
	bool finishCondition = false;
    SDL_Window* window = NULL;
    SDL_Surface* screenSurface = NULL; 

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) { 
		std::cout << "Błąd inicjalizacji: SDL_Error: " << SDL_GetError() << std::endl; 
	} else {
		window = SDL_CreateWindow( "Okienko SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN ); 
		if( window == NULL ) { 
			std::cout << "Błąd tworzenia okna: SDL_Error: " << SDL_GetError() << std::endl; 
		} else {
			screenSurface = SDL_GetWindowSurface( window ); 
		} 
	}
    
    while (!finishCondition) {
		SDL_Event e;
		SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xAA,  rand() % 0x0ff ));
		SDL_UpdateWindowSurface( window ); 
		// ważna sprawa: należy zebrać wszystkie zdarzenia od ostatniej klatki animacji
		while( SDL_PollEvent( &e ) != 0 ) { 
			if( e.type == SDL_QUIT ) { 
				finishCondition = true;
			} else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					finishCondition = true;
					break;
				case SDLK_q:
					finishCondition = true;
					break;
				default:
					break;
				}
			}
		}
		std::this_thread::sleep_for (std::chrono::milliseconds(33));
	}
    
    //Destroy window 
    SDL_DestroyWindow( window );
    //Quit SDL 
    SDL_Quit(); 
    return 0; 
}
