#include <SDL.h>

#include <vector>
#include <tuple>
#include <map>
#include <cmath>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>
#include <stdexcept>
	
	/*
	+---------------+
	|               |
	|               |
	|0,0            |
	+---------------+
	kafelki rozmiaru 32x32
	*/


#include <spriteengine.hpp>
#include <gamemap.hpp>
#include <player.hpp>

#include <veclib/veclib.hpp>


using std::vector;
using std::map;
using std::tuple;
using std::make_tuple;
using std::function;
using std::string;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::system_clock;


class Game {
	public:
	
	SDL_Surface * s_background;
	SDL_Texture * t_background;

	GameMap gameMap;
	Player player;

	void draw(double t);
	int input();
	void physics(double dt);
	Game(const map<string,string> config = map<string,string>());
	
	virtual ~Game() ;
};


void Game::draw(double t) {
	spriteEngine.startFrame();
	
	SDL_Rect dstrect = {
		(int)(-spriteEngine.cameraPosition[0]*unitSize-spriteEngine.screenSize[0])>>1,  
		(int)(spriteEngine.screenSize[1] + spriteEngine.cameraPosition[1]*unitSize-spriteEngine.screenSize[1]*3)>>1,
		(int)spriteEngine.screenSize[0]*4,(int)spriteEngine.screenSize[1]*4};
	SDL_RenderCopy(spriteEngine.renderer, t_background, NULL, &dstrect);
	
	gameMap.draw(t);		
	player.draw(t);
	
	spriteEngine.finishFrame();
};

int Game::input() {
	int ret = 0;
	SDL_Event e;
	while( SDL_PollEvent( &e ) != 0 ) { 
		if( e.type == SDL_QUIT ) { 
			ret |= 1;
		} else if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.sym == SDLK_ESCAPE) ret |= 1;
		} else if (e.type == SDL_KEYUP) {
			//if (keyUpHandlers.count(e.key.keysym.sym) > 0) keyUpHandlers[e.key.keysym.sym](e);
		}
	}
	
	// sterowanie
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	player.acceleration = {0,0};
	
	// sprawdzamy klawisze
	if (player.isOnGround(gameMap)) {
		if (state[SDL_SCANCODE_LEFT])   player.acceleration[0] -= 20.0;
		if (state[SDL_SCANCODE_RIGHT])  player.acceleration[0] += 20.0;
		if (state[SDL_SCANCODE_UP])     player.speed[1] = 9.0;
		//if (state[SDL_SCANCODE_DOWN])   player.acceleration[1] -= 20.0;
	} else {
		if (state[SDL_SCANCODE_LEFT])   player.acceleration[0] -= 2.0;
		if (state[SDL_SCANCODE_RIGHT])  player.acceleration[0] += 2.0;
	}
	return ret;
};
void Game::physics(double dt) {
	spriteEngine.cameraSpeed = (player.position-spriteEngine.cameraPosition)*0.96;
	spriteEngine.cameraPosition += spriteEngine.cameraSpeed*dt;
	player.physics(gameMap, dt);
};
Game::Game(const map<string,string> config) {
	std::tie(s_background, t_background) = spriteEngine.getResource("img/background.bmp");
	gameMap = GameMap("map.bmp");
};

Game::~Game() {
};





int main( int argc, char* args[] ) { 
	Game game;
	bool finishCondition = false;
    
    auto prevTime = std::chrono::system_clock::now();
    auto t0 = prevTime; // zero time
    unsigned long int frame = 0;
    double t = 0;
	while (!finishCondition) {
		double dt=0;
		std::chrono::duration<double> cdt; // frame time
		game.draw(t);
		auto now = system_clock::now();
		cdt = now - prevTime; // przyrost czasu w sekundach
		dt = cdt.count();
		prevTime = now;
		// std::cout << "FPS: " << (1.0/cdt.count()) << std::endl;
		if (game.input() != 0) finishCondition = true;
		game.physics(dt);
		t+= dt;
	}
	return 0; 
}
