#include <SDL.h>

#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>


using std::vector;
using std::map;
using std::function;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;


map<SDL_Keycode,function < void (SDL_Event &) > > keyDownHandlers;
map<SDL_Keycode,function < void (SDL_Event &) > > keyUpHandlers;

template < typename T > 
vector < T > operator +(const vector < T > &a, const vector < T > &b) {
	vector < T > r(a.size());
	for (int i = 0; i < a.size(); i++) r[i] = a[i] + b[i];
	return r;
}
template < typename T > 
vector < T > operator -(const vector < T > &a, const vector < T > &b) {
	vector < T > r(a.size());
	for (int i = 0; i < a.size(); i++) r[i] = a[i] - b[i];
	return r;
}
template < typename T > 
vector < T > &operator +=(vector < T > &a, const vector < T > &b) {
	for (int i = 0; i < a.size(); i++) a[i] += b[i];
	return a;
}
template < typename T > 
vector < T > &operator *=(vector < T > &a, const vector < T > &b) {
	for (int i = 0; i < a.size(); i++) a[i] *= b[i];
	return a;
}
template < typename T > 
vector < T > &operator *=(vector < T > &a, const T  &b) {
	for (int i = 0; i < a.size(); i++) a[i] *= b;
	return a;
}
	

int main( int argc, char* args[] ) { 
	bool finishCondition = false;
	

    SDL_Init( SDL_INIT_EVERYTHING ); 
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);
    vector <double > p(2); // pozycja
    vector <double > dp(2); // predkosc
    vector <double > ddp(2); // przyspieszenie
    p[0] = 0; p[1] = 0;
    dp[0] = 0; dp[1] = 0;
    ddp[0] = 0; ddp[1] = 0;
    
	keyDownHandlers[SDLK_ESCAPE] = [&](SDL_Event &e){finishCondition=true;};
    
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
        // delay
        sleep_for (milliseconds(33));

        // zdarzenia
		SDL_Event e;
		while( SDL_PollEvent( &e ) != 0 ) { 
			if( e.type == SDL_QUIT ) { 
				finishCondition = true;
			} else if (e.type == SDL_KEYDOWN) {
				if (keyDownHandlers.count(e.key.keysym.sym) > 0) keyDownHandlers[e.key.keysym.sym](e);
			} else if (e.type == SDL_KEYUP) {
				if (keyUpHandlers.count(e.key.keysym.sym) > 0) keyUpHandlers[e.key.keysym.sym](e);
			}
		}
		
		// sterowanie
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		// zerujemy aktuatory
		ddp[0] = 0;
		ddp[1] = 0;
        // sprawdzamy klawisze
		if (state[SDL_SCANCODE_LEFT]) ddp[0] = -0.01;
		if (state[SDL_SCANCODE_RIGHT]) ddp[0] = 0.01;
		if (state[SDL_SCANCODE_UP]) ddp[1] = 0.01;
		if (state[SDL_SCANCODE_DOWN]) ddp[1] = -0.01;

		//fizyka
		dp += ddp;
		p += dp;
		dp *= 0.9;

		
	}
   
    SDL_DestroyWindow( window );
    SDL_Quit(); 
    return 0; 
}
