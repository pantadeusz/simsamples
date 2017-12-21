/**
 * This is the simple hello world for SDL2.
 *
 * The example tries to take care of resource allocation and deallocation
 * using smart pointers.
 */

#include "vid_sdl.hpp"

#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include <random>


#include <iostream>


using position_t = std::array<double, 2>;
position_t operator+( const position_t &a, const position_t &b ) {
	return {a[0] + b[0], a[1] + b[1]};
}
position_t operator-( const position_t &a, const position_t &b ) {
	return {a[0] - b[0], a[1] - b[1]};
}
position_t operator*( const position_t &a, const double &b ) {
	return {a[0]*b, a[1]*b};
}
position_t operator*( const position_t &a, const position_t &b ) {
	return {a[0]*b[0], a[1]*b[1]};
}
position_t operator/( const position_t &a, const position_t &b ) {
	return {a[0] / b[0], a[1] / b[1]};
}

class Particle {
public:
	position_t position;
	position_t velocity;
	position_t accel;

	double ttl; // czas zycia

	Particle( const position_t &p0, const position_t &v0 = {0, 0}, const position_t &a0 = {0, 0} ): position( p0 ), velocity( v0 ), accel( a0 ) {
		ttl = 1;
	}

	void update( std::chrono::duration<double> &dt ) {
		double dts = dt.count();
		position = position + velocity * dts + accel * dts * dts * 0.5;
		velocity = velocity + accel * dts;
		ttl -= dts;
	}

};

void calculateParticles( std::vector < Particle >  &particles0, std::chrono::duration<double> &dt ) {
	std::vector < bool> dels( particles0.size() );
	int i = 0;
	auto particles = particles0;
	for ( auto & p : particles ) {
		p.update( dt );
		dels[i] = p.ttl < 0;
	}
	particles0.clear();
	for ( int i = 0; i < particles.size(); i++ ) {
		if ( !dels[i] ) particles0.push_back( particles[i] );
	}
}


using namespace sgd;



int main( ) { // int argc, char **argv ) {

	auto window = init_window( 640, 480 );
	auto renderer = init_renderer( window );

	Particle particle( {160, 100}, {10, -50}, {0, 10} );

	auto particle_tex = create_texture( renderer, 16, 16 );
	std::unique_ptr < Uint32 > pixels ( new Uint32[16 * 16] );
	for ( int x = 0; x < 16; x++ ) {
		for ( int y = 0; y < 16; y++ ) {
//			pixels.get()[y * 16 + x] = ( 0x0550000 + ( int )( sin( x / 3.0 ) * 127 + 128 ) +  ( ( int )( cos( y / 3.0 ) * 127 + 128 ) << 8 ) ) & 0x0ffffff;
			pixels.get()[y * 16 + x] = ( ( int )( sin( x / 3.0 + y / 6 ) * 127 + 128 ) + ( ( int )( sin( x / 3.0 + y / 6 ) * 127 + 128 ) << 8 )  + ( ( int )( sin( x / 3.0 + y / 6 ) * 127 + 128 ) << 16 ) ) & 0x0ffffff;
			double r = std::sqrt( ( x - 8 ) * ( x - 8 ) + ( y - 8 ) * ( y - 8 ) );
			int a = 0;
			if ( r <= 8 ) a = 255 * ( 8 - a );
			pixels.get()[y * 16 + x] += a << 24;
		}
	}
	SDL_UpdateTexture( particle_tex.get(), NULL, pixels.get(), 16 * sizeof( Uint32 ) );

	std::chrono::duration<double> dt( 0.01 ); // w sekundach

	std::vector < Particle > particles;
	particles.reserve( 30000 );


	bool updating = false;
	auto prevTime = std::chrono::high_resolution_clock::now(); // w sekundach
	position_t scaleFactor = {10, 10};
	double timeToShoot = 0;
	double timeToSpawnParicle = 0.6;

    std::random_device randomDev;
    // Choose a random mean between 1 and 6
    std::default_random_engine randomEngine(randomDev());
    std::uniform_int_distribution<int> uniform_dist_x(0, 640);
    std::uniform_int_distribution<int> uniform_dist_y(0, 480);
	for ( bool game_active = true ; game_active; ) {
		SDL_Event event;
		while ( SDL_PollEvent( &event ) ) {
			switch ( event.type ) {
			case SDL_QUIT:
				game_active = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				std::cout << "Button: L:" << ( event.button.button == SDL_BUTTON_LEFT ) << " R:" << ( event.button.button == SDL_BUTTON_RIGHT ) << std::endl;
				updating = true;
				break;
			case SDL_MOUSEBUTTONUP:
				std::cout << "      : L:" << ( event.button.button == SDL_BUTTON_LEFT ) << " R:" << ( event.button.button == SDL_BUTTON_RIGHT ) << std::endl;
				updating = false;
				break;
			case SDL_MOUSEMOTION:
				std::cout << "motion: " << event.motion.x << " " << event.motion.y << std::endl;
				if ( updating && ( timeToShoot <= 0 ) ) {
					position_t np = {event.motion.x - 8, event.motion.y - 8};
					np = np / scaleFactor;
					Particle particle( np, {10, -20}, {0, 10} ) ;
					particle.ttl = 20;
					particles.push_back( particle );
					timeToShoot = 0.1;
				}
				break;
			}
		}

		// fizyka

		if ( timeToSpawnParicle <= 0 ) {
			position_t p = {uniform_dist_x(randomEngine),uniform_dist_y(randomEngine)};
			p = p / scaleFactor;
			Particle particle( p, {-10, 0}, {0, 0} ) ;
			particle.ttl = 15;
			particles.push_back( particle );
			timeToSpawnParicle = 0.3;
		}

		calculateParticles( particles, dt ) ;
		if ( timeToShoot > 0 )timeToShoot -= dt.count();
		if ( timeToSpawnParicle > 0 ) timeToSpawnParicle -= dt.count();

		
		// grafika


		SDL_SetRenderDrawColor( renderer.get(), 0, 0, 0, 0 );
		SDL_RenderClear( renderer.get() );

		SDL_SetRenderDrawColor( renderer.get(), 255, 0, 0, 255 );
		//SDL_RenderDrawPoint( renderer.get(), 10, 10 );
		for ( auto & p : particles ) {
			auto pos = p.position * scaleFactor;
			SDL_Rect destRectForFace = {
				.x = pos[0],
				.y = pos[1],
				.w = 16,
				.h = 16
			};
			SDL_RenderCopyEx( renderer.get(), particle_tex.get(), NULL,  &destRectForFace,  destRectForFace.x,  NULL, SDL_FLIP_NONE );
		}
		SDL_RenderPresent( renderer.get() );

		std::this_thread::sleep_until( prevTime + dt );
		prevTime = std::chrono::high_resolution_clock::now();
	}
	return 0;
}
