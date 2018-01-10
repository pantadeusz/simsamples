#include "p_particles.hpp"
#include <iostream>

namespace sgd {


Particle::Particle( const position_t &p0, const position_t &v0, const position_t &a0 ): position( p0 ), velocity( v0 ), accel( a0 ) {
	ttl = 1;
	on_update = []( Particle &p, const duration_t &dt ) {
	    double dts = dt.count();
	    p.position = p.position + p.velocity * dts + p.accel * dts * dts * 0.5;
	    p.velocity = p.velocity + p.accel * dts;
	    p.ttl -= dts;
		return std::vector < Particle >();
	};
	draw_particle = []( const Particle & ) {};
	on_interaction = []( Particle &, const Particle &, const duration_t & ){};
}

void draw_particles( const std::vector < Particle > &particles0 ) {
    for (auto &p:particles0) p.draw_particle(p);
}

std::vector < Particle > calculate_particles( const std::vector < Particle >  &particles0, duration_t &dt ) {
    std::vector < bool> dels( particles0.size() );
	auto i = 0;
	std::vector < Particle > particles = particles0;
	std::vector < Particle > toAdd; // particles to add
	for ( auto & p : particles ) {
		auto t = p.on_update( p, dt );
        toAdd.insert( toAdd.end(), t.begin(), t.end() );
		dels[i] = p.ttl < 0;
		i++;
	}
    std::vector < Particle > ret;
    ret.reserve(particles.size());
	for ( int i = 0; i < particles.size(); i++ ) {
		if ( !dels[i] ) ret.push_back( particles[i] );
	}
	ret.insert( ret.end(), toAdd.begin(), toAdd.end() );
    return ret;
}

}
