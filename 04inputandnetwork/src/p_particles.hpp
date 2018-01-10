#ifndef __P_PARTICLES__
#define __P_PARTICLES__

#include "p_structures.hpp"
#include <functional>
#include <chrono>
#include <vector>

namespace sgd {
class Particle {
public:
	position_t position;
	position_t velocity;
	position_t accel;

	double ttl; // czas zycia

	std::function < std::vector < Particle > ( Particle & ) > onExitScreen;
	std::function < void ( Particle & ) > drawParticle;

	Particle( const position_t &p0, const position_t &v0 = {0, 0}, const position_t &a0 = {0, 0} );
	void update( std::chrono::duration<double> &dt );
};

void calculateParticles( std::vector < Particle >  &particles0, std::chrono::duration<double> &dt );


}

#endif
