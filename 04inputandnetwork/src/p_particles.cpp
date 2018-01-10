#include "p_particles.hpp"

namespace sgd {


Particle::Particle( const position_t &p0, const position_t &v0, const position_t &a0 ): position( p0 ), velocity( v0 ), accel( a0 ) {
	ttl = 1;
	onExitScreen = []( Particle & ) {
		return std::vector < Particle >();
	};
	drawParticle = []( Particle & ) {};
}

void Particle::update( std::chrono::duration<double> &dt ) {
	double dts = dt.count();
	position = position + velocity * dts + accel * dts * dts * 0.5;
	velocity = velocity + accel * dts;
	ttl -= dts;
}

void calculateParticles( std::vector < Particle >  &particles0, std::chrono::duration<double> &dt ) {
	std::vector < bool> dels( particles0.size() );
	auto i = 0;
	auto particles = particles0;
	std::vector < Particle > toAdd;
	for ( auto & p : particles ) {
		p.update( dt );
		if ( ( p.position[0] < 0 ) || ( p.position[1] < 0 ) || ( p.position[0] > 64 ) || ( p.position[1] > 48 ) ) {
			auto t = p.onExitScreen( p );
			toAdd.insert( toAdd.end(), t.begin(), t.end() );
		}
		dels[i] = p.ttl < 0;
		i++;
	}
	particles0.clear();
	for ( int i = 0; i < particles.size(); i++ ) {
		if ( !dels[i] ) particles0.push_back( particles[i] );
	}
	particles0.insert( particles0.end(), toAdd.begin(), toAdd.end() );
}

}
