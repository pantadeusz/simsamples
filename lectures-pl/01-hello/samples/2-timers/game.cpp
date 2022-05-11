#include <SDL.h>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>

double fps = 60;  // docelowy

int main( int argc, char* args[] ) { 
	std::function <bool ()>  finishCondition = [](){return false;};
    SDL_Init( SDL_INIT_EVERYTHING ); 
    SDL_Window* window = NULL;
    SDL_Surface* screenSurface = NULL; 

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) { 
		std::cout << "Błąd inicjalizacji: SDL_Error: " << SDL_GetError() << std::endl; 
	} else {
		window = SDL_CreateWindow( "Okno SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN ); 
		if( window == NULL ) { 
			std::cout << "Błąd tworzenia okna: SDL_Error: " << SDL_GetError() << std::endl; 
		} else {
			screenSurface = SDL_GetWindowSurface( window ); 
		} 
	}
    
    auto prevTime = std::chrono::steady_clock::now();
    auto t0 = prevTime; // zero time
    unsigned long int frame = 0;
    while (!finishCondition()) {
		SDL_Event e;
		// render scene
		SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xAA,  0x0ff ));
		SDL_UpdateWindowSurface( window ); 
		// ważna sprawa: należy zebrać wszystkie zdarzenia od ostatniej klatki animacji
		while( SDL_PollEvent( &e ) != 0 ) { 
			if( e.type == SDL_QUIT ) { 
				finishCondition = []{return true;};
			} 
		}
		SDL_Delay(rand()%14);

		//std::this_thread::sleep_until(prevTime+1s);
		
		// Obliczamy ile czasu minęło od ostatniej klatki animacji
//		std::cout << "Wait: " << ((int)(10000000.0*(1.0/fps - (((std::chrono::duration<double>)(std::chrono::steady_clock::now() - prevTime)).count())))) << "\n";
//		std::this_thread::sleep_for (std::chrono::microseconds((int)(1000000.0*((double)1.0/fps - (((std::chrono::duration<double>)(std::chrono::steady_clock::now() - prevTime)).count()))))); // to jest OK przyrostowo
		std::this_thread::sleep_for (std::chrono::microseconds((int)(1000000.0*(((double)frame)/fps - (((std::chrono::duration<double>)(std::chrono::steady_clock::now() - t0)).count()))))); // to jest OK przyrostowo

		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<double> dt = now - prevTime; // przyrost czasu w sekundach
		prevTime = now;
		
		// calculate physics
		
		// show fps
		//std::cout << "T: " << (((std::chrono::duration<double>)(now - t0)).count()) <<  " FPS: " << (1.0/dt.count()) << std::endl;
		std::cout << " FPS: " << (1.0/dt.count()) << std::endl;
		frame ++;
	}
    
    //Destroy window 
    SDL_DestroyWindow( window );
    //Quit SDL 
    SDL_Quit(); 
    return 0; 
}
