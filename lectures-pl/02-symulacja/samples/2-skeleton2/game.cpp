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
using std::chrono::microseconds;
using std::chrono::steady_clock;


map<SDL_Keycode,function < void (SDL_Event &) > > keyDownHandlers;
map<SDL_Keycode,function < void (SDL_Event &) > > keyUpHandlers;

enum ParticleVariant {
	bullet, particle
};

class Particle {
	public:
	vector < double > p;  // pozycja
	vector < double > v;  // predkosc
	double ttl;           // czas zycia (w sekundach)
	int variant;          // rodzaj czasteczki
	Particle() {
		p = {0,0};
		v = {0,0};
		ttl = 0;
		variant = bullet;
	};
	Particle(const vector < double > &p0, const vector < double > &v0, const double variant0 = bullet,const double ttl0 = 0):p(p0),v(v0),ttl(ttl0),variant(variant0) {
	};
};


int main( int argc, char* args[] ) { 
	bool finishCondition = false;
	
	SDL_Init( SDL_INIT_EVERYTHING ); 
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);
	vector < Particle > particles;
	particles.push_back(Particle({10,10},{1,1},bullet,10));
	double fps=60.0; // 60FPS
	double dt=1.0/fps; // 60FPS
	vector <double > p(2); // pozycja
	vector <double > dp(2); // predkosc
	vector <double > ddp(2); // przyspieszenie
	p[0] = 0; p[1] = 0;
	dp[0] = 0; dp[1] = 0;
	ddp[0] = 0; ddp[1] = 0;
	keyDownHandlers[SDLK_ESCAPE] = [&](SDL_Event &e){finishCondition=true;};

    auto prevTime = std::chrono::steady_clock::now();
    auto t0 = prevTime; // zero time
    unsigned long int frame = 0;

	while (!finishCondition) {
		std::chrono::duration<double> cdt; // frame time
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
		if (state[SDL_SCANCODE_LEFT]) ddp[0] = -10;
		if (state[SDL_SCANCODE_RIGHT]) ddp[0] = 10;
		if (state[SDL_SCANCODE_UP]) ddp[1] = 10;
		if (state[SDL_SCANCODE_DOWN]) ddp[1] = -10;


// A:
		sleep_for (milliseconds((int)(dt*1000.0)));
// /A
// B:
//		frame++;
//		sleep_for (microseconds((int)(1000000.0*(((double)frame)/fps - (((std::chrono::duration<double>)(steady_clock::now() - t0)).count()))))); // to jest OK przyrostowo
// /B
		auto now = steady_clock::now();
		cdt = now - prevTime; // przyrost czasu w sekundach
// C:
//		dt = cdt.count();
// /C
		prevTime = now;
		std::cout << "FPS: " << (1.0/cdt.count()) << std::endl;



		//fizyka
		dp += ddp*dt*dt;
		p += dp*dt;
		dp = dp - dp*0.1*dt;


	}

	SDL_DestroyWindow( window );
	SDL_Quit(); 
	return 0; 
}
