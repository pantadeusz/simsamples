#include "p_particles.hpp"
#include <iostream>

namespace sgd {

std::function < std::vector < Particle > ( Particle &, const duration_t & ) > Particle::get_default_update_f() {
	return []( Particle & p, const duration_t &dt ) {
		double dts = dt.count();
		p.position = p.position + p.velocity * dts + p.accel * dts * dts * 0.5;
		p.velocity = p.velocity + p.accel * dts;
		p.ttl -= dts;
		return std::vector < Particle >();
	};
}

void Particle::init( const position_t &p0, const position_t &v0, const position_t &a0 ) {
	position = p0;
	velocity = v0;
	accel = a0;
	ttl = 1;
	on_update = Particle::get_default_update_f();
	draw_particle = []( const Particle & ) {};
	on_interaction = []( Particle &, const Particle &, const duration_t & ) {};
}

void draw_particles( const std::vector < Particle > &particles0 ) {
	for ( auto &p : particles0 ) p.draw_particle( p );
}

std::vector < Particle > calculate_particles( const std::vector < Particle >  &particles0, duration_t &dt ) {
	std::vector < bool> dels( particles0.size() );
	auto i = 0;
	auto particles = particles0;
	std::vector < Particle > new_particles; // czasteczki do dopisania do tablicy
	for ( auto & p : particles ) {
		// interakcje miedzy czasteczkami
		for ( auto & p2 : particles) {
			if (&p != &p2) p.on_interaction(p, p2, dt);
		}
		// aktualizacja czasteczki i ewentualnie wygenerowanie nowych
		auto nplist = p.on_update( p, dt );
		// dodanie nowych czasteczek
		new_particles.insert( new_particles.end(), nplist.begin(), nplist.end() );
		dels[i] = p.ttl < 0;
		i++;
	}
	std::vector < Particle > ret;
	ret.reserve( particles.size() );
	for ( unsigned i = 0; i < particles.size(); i++ ) {
		if ( !dels[i] ) ret.push_back( particles[i] );
	}
	ret.insert( ret.end(), new_particles.begin(), new_particles.end() );
	return ret;
}

}
