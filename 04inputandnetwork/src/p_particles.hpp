#ifndef __P_PARTICLES__
#define __P_PARTICLES__

#include "p_structures.hpp"
#include <functional>
#include <chrono>
#include <vector>

namespace sgd {
using duration_t = std::chrono::duration<double>; //std::chrono::milliseconds;//std::chrono::duration<double,std::ratio<1>>; //std::chrono::duration_cast<std::chrono::milliseconds>

class Particle {
public:
	position_t position;
	position_t velocity;
	position_t accel;

	double ttl; // czas zycia, jesli dodatni, to czasteczka zyje

// w momencie aktualizacji czasteczki
	std::function < std::vector < Particle > ( Particle &, const duration_t & ) > on_update;

// implementacja oddzialywan miedzy czasteczkami. Dla kazdej pozostalej czasteczki jest to wywolywane
	std::function < void ( Particle &, const Particle &, const duration_t & ) > on_interaction;

// w momencie rysowania czasteczki
	std::function < void ( const Particle & ) > draw_particle; 

// konstruktor
	void init( const position_t &p0, const position_t &v0 = {0, 0}, const position_t &a0 = {0, 0} );

// funkcje statyczne ----------
	static std::function < std::vector < Particle > ( Particle &, const duration_t & ) > get_default_update_f();
};


std::vector < Particle > calculate_particles( const std::vector < Particle >  &particles0, duration_t &dt );

void draw_particles( const std::vector < Particle > &particles0);


}

#endif
