/*

  helper functions and data structures that allow for convinient operating on vectors and scalars.

  Tadeusz Puźniakowski 2018
*/
#ifndef __P_STRUCTURES__
#define __P_STRUCTURES__

#include <array>

namespace sgd {

using position_t = std::array<double, 2>;
position_t operator+( const position_t &a, const position_t &b );
position_t operator-( const position_t &a, const position_t &b );
position_t operator*( const position_t &a, const double &b );
position_t operator*( const position_t &a, const position_t &b );
position_t operator/( const position_t &a, const position_t &b );

}

#endif
