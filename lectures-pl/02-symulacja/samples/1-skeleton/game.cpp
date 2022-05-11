#include <SDL.h>

#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>

#include <veclib/veclib.hpp>

using std::vector;
using std::map;
using std::function;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;

int main( int argc, char* args[] ) { 
	bool finishCondition = false;
	
	SDL_Init( SDL_INIT_EVERYTHING ); 
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);

	double fps = 60;
	double dt=1.0/fps; // 60FPS
	vector <double > p(2); // pozycja
	vector <double > dp(2); // predkosc
	vector <double > ddp(2); // przyspieszenie
	p[0] = 0; p[1] = 0;
	dp[0] = 0; dp[1] = 0;
	ddp[0] = 0; ddp[1] = 0;
	
	while (!finishCondition) {
		// grafika
		// glVertex3f(p[0], p[1], 0.0);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		for (double px = -0.01; px < 0.01; px+= 0.001) 
		for (double py = -0.01; py < 0.01; py+= 0.001) 
		SDL_RenderDrawPoint(renderer, (p[0]+px)*400.0 + 400.0, -(p[1]+py)*300.0 + 300.0); //Renders on middle of screen.
		SDL_RenderDrawPoint(renderer, p[0]*400.0 + 400.0, -p[1]*300.0 + 300.0); //Renders on middle of screen.
		SDL_RenderPresent(renderer);	

        // zdarzenia
		SDL_Event e;
		while( SDL_PollEvent( &e ) != 0 ) { 
			if( e.type == SDL_QUIT ) { 
				finishCondition = true;
			} else if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE) finishCondition = true;
			}
		}
		
		// sterowanie
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		// zerujemy aktuatory
		ddp[0] = 0;
		ddp[1] = 0;
		// sprawdzamy klawisze
		if (state[SDL_SCANCODE_LEFT]) ddp[0] = -10;
		if (state[SDL_SCANCODE_RIGHT]) ddp[0] = 10;
		if (state[SDL_SCANCODE_UP]) ddp[1] = 10;
		if (state[SDL_SCANCODE_DOWN]) ddp[1] = -10;

		sleep_for (milliseconds((int)(dt*1000.0)));

		//fizyka
		dp += ddp*dt*dt;
		p += dp*dt;
		dp = dp - dp*0.1*dt;

	}

	SDL_DestroyWindow( window );
	SDL_Quit(); 
	return 0; 
}
