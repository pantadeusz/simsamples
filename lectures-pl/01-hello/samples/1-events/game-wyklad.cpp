#include <SDL.h>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>




int main( int argc, char* args[] ) { 
	bool finishCondition = false;
    SDL_Window* window = NULL;
    SDL_Surface* screenSurface = NULL; 

    SDL_Init( SDL_INIT_VIDEO );
	window = SDL_CreateWindow( "Okienko SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN ); 
	screenSurface = SDL_GetWindowSurface( window ); 
	
	double x = 0, y = 0;   // pozycja
	double dx = 0, dy = 0;  // prędkość
	double ddx = 0, ddy = 0; // przyśpieszenie 
	double dt = 1.0/120.0; // przyrost czasu
	
    while (!finishCondition) {
		SDL_Event e;
		SDL_FillRect( screenSurface, NULL, 
		SDL_MapRGB( screenSurface->format, 0, 0, 0 ));
		SDL_Rect rect;
		rect.x = x;
		rect.y = y;
		rect.w = 10;
		rect.h = 10;
		
		SDL_FillRect( screenSurface, &rect, 
			SDL_MapRGB( screenSurface->format, 0, 250, 0 )
		);
		
		
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

		const Uint8 *state = SDL_GetKeyboardState(NULL);
		ddx = ddy = 0;
		if (state[SDL_SCANCODE_LEFT]) ddx = -100;
		if (state[SDL_SCANCODE_RIGHT]) ddx = 100;
		if (state[SDL_SCANCODE_UP]) ddy = -100;
		if (state[SDL_SCANCODE_DOWN]) ddy = 100;

		std::cout << ":" << x << "," << y << "\n";
		dx *= 0.99; 
		dy *= 0.99;
		dx += ddx*dt;
		dy += ddy*dt;
		x += dx*dt;
		y += dy*dt;
		std::this_thread::sleep_for (std::chrono::milliseconds((int)(1000.0*dt)));
	}
    
    //Destroy window 
    SDL_DestroyWindow( window );
    //Quit SDL 
    SDL_Quit(); 
    return 0; 
}
