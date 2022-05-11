#include <SDL.h>
#include <GL/gl.h>

#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>


using std::vector;
using std::map;
using std::function;

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
    auto window = SDL_CreateWindow( "Okienko SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL ); 
    SDL_GL_CreateContext(window);
    
    vector <double > p(2); // pozycja
    vector <double > dp(2); // predkosc
    vector <double > ddp(2); // przyspieszenie
    p[0] = 0; p[1] = 0;
    dp[0] = 0; dp[1] = 0;
    ddp[0] = 0; ddp[1] = 0;
    
	keyDownHandlers[SDLK_ESCAPE] = [&](SDL_Event &e){finishCondition=true;};
    
    while (!finishCondition) {
		// grafika
		glClear(GL_COLOR_BUFFER_BIT);
		glBegin(GL_POLYGON);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(0.5, 0.0, 0.0);
			glVertex3f(p[0], p[1], 0.0);
			glVertex3f(0.0, 0.5, 0.0);
		glEnd();
		//glFlush();
		SDL_GL_SwapWindow(window);

        // delay
        std::this_thread::sleep_for (std::chrono::milliseconds(33));

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
