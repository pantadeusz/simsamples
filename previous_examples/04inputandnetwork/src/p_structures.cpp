#include "p_structures.hpp"

namespace sgd {


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

}
