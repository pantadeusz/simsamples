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



using vec_t = std::array<double, 2>;
using ball_t = std::pair<vec_t, vec_t>;

void draw_ball(const ball_t &ball,const double r, std::shared_ptr<SDL_Renderer> renderer) {

	for (double a = -M_PI; a < M_PI; a+= 0.01) {
                SDL_SetRenderDrawColor(renderer.get(), 255.0*(a/(M_PI))*(a/(M_PI)), 0, 255, SDL_ALPHA_OPAQUE);
				SDL_RenderDrawLine(renderer.get(),
                ball.first[0],
                ball.first[1],
                ball.first[0]+r*::sin(a),
                ball.first[1]+r*::cos(a));
	}
}

vec_t operator-(const vec_t &a, const vec_t &b) {
	return {a[0]-b[0],a[1]-b[1]};
}
vec_t operator+(const vec_t &a, const vec_t &b) {
	return {a[0]+b[0],a[1]+b[1]};
}
vec_t operator*(const vec_t &a, const vec_t &b) {
	return {a[0]*b[0],a[1]*b[1]};
}
vec_t operator*(const vec_t &a, const double b) {
	return {a[0]*b,a[1]*b};
}
vec_t operator/(const vec_t &a, const double b) {
	return {a[0]/b,a[1]/b};
}
double length(const vec_t &d_) {
	auto d = d_*d_;
	return std::sqrt(d[0]+d[1]);
}


int main( ) { // int argc, char **argv ) {

	auto window = init_window();
	auto renderer = init_renderer( window );

	std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(1.0, 10.0);

	std::vector < ball_t > balls = {{{20, 100},{0,0}},{{300, 100},{-100,0}},{{150, 150},{0,0}},{{150, 50},{0,0}}};

	double dt = 0.01;
	for ( bool game_active = true ; game_active; ) {
		// events
		SDL_Event event;
		while ( SDL_PollEvent( &event ) ) {
			switch (event.type) {
				case SDL_QUIT: game_active = false; break;
				case SDL_MOUSEMOTION: 
					balls.at(0).second[0] = (balls.at(0).second[0] + 100*(event.motion.x - balls.at(0).first[0]))/2.0;
					balls.at(0).second[1] = (balls.at(0).second[1] + 100*(event.motion.y - balls.at(0).first[1]))/2.0;

					balls.at(0).first[0] = event.motion.x;
					balls.at(0).first[1] = event.motion.y;
				break;
			}
		}
		// physics

		// calculate collisions
		std::vector<ball_t> new_balls;
		for (const auto &active : balls) {
			auto active_n = active;
			for (const auto &test : balls) {
				if (&test != &active) {
					vec_t a_p,a_v,t_p,t_v;
					std::tie(a_p,a_v) = active;
					std::tie(t_p,t_v) = test;

					double l = length(a_p-t_p);
					if (l < 20.0) {
						if (l < 10.0) l = 8; // no infinite movements
						auto v = (t_p - a_p)/l; // 1
						
						active_n.second = active_n.second + t_v;
						//std::cout << "collision " << a_v[0] << " " << a_v[1] << "\n";
					}
				}
			}
			new_balls.push_back(active_n);
		}
		balls = new_balls;
		// move them away from each other
		for (auto &active : balls) {
			for (const auto &test : balls) {
				if (&test != &active) {
					vec_t a_p,a_v,t_p,t_v;
					std::tie(a_p,a_v) = active;
					std::tie(t_p,t_v) = test;

					double l = length(a_p-t_p);
					if (l < 20.0) {
						auto v = (t_p - a_p)/l; // 1
						a_p = t_p - v * 22;
						std::cout << "collision " << a_v[0] << " " << a_v[1] << "\n";
					}
					active = {a_p,a_v};
				}
			}
			if (active.first[0] < 0) {
				active.first[0] = 0;
				active.second[0] *= -1;
			}
			if (active.first[0] > 319) {
				active.first[0] = 319;
				active.second[0] *= -1;
			}
			if (active.first[1] < 0) {
				active.first[1] = 0;
				active.second[1] *= -1;
			}
			if (active.first[1] > 199) {
				active.first[1] = 199;
				active.second[1] *= -1;
			}
		}
		// apply accel
		for (auto &b : balls) {
			if (&(balls[0]) == &b) {
			} else {
				b.first[0] = b.first[0] + b.second[0]*dt;
				b.first[1] = b.first[1] + b.second[1]*dt;
			}
				b.second[0] = b.second[0] - b.second[0]*1*dt;
				b.second[1] = b.second[1] - b.second[1]*1*dt;
		}
		// video
        SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear( renderer.get() );
		for (auto &b:balls) draw_ball(b,10.5,renderer);


		SDL_RenderPresent( renderer.get() );
		SDL_Delay( 1000.0*dt );
	}
	return 0;
}
