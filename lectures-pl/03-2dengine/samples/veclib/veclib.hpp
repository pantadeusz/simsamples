/**
 * Tadeusz Puźniakowski, 2016
 * 
This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 * */

#ifndef __VECLIB_TP___
#define __VECLIB_TP___

#include <vector>

// suma wektorow
template < typename T > 
std::vector < T > operator +(const std::vector < T > &a, const std::vector < T > &b) {
	std::vector < T > r(a.size());
	for (int i = 0; i < a.size(); i++) r[i] = a[i] + b[i];
	return r;
}
// roznica wektorow
template < typename T > 
std::vector < T > operator -(const std::vector < T > &a, const std::vector < T > &b) {
	std::vector < T > r(a.size());
	for (int i = 0; i < a.size(); i++) r[i] = a[i] - b[i];
	return r;
}
// suma wektorow
template < typename T > 
std::vector < T > &operator +=(std::vector < T > &a, const std::vector < T > &b) {
	for (int i = 0; i < a.size(); i++) a[i] += b[i];
	return a;
}
// roznica wektorow
template < typename T > 
std::vector < T > &operator -=(std::vector < T > &a, const std::vector < T > &b) {
	for (int i = 0; i < a.size(); i++) a[i] -= b[i];
	return a;
}
// pomnozenie wektora przez liczbe
template < typename T > 
std::vector < T > &operator *=(std::vector < T > &a, const T &b) {
	for (int i = 0; i < a.size(); i++) a[i] *= b;
	return a;
}
// pomnozenie wektora przez liczbe
template < typename T > 
std::vector < T > operator *(const std::vector < T > &a, const T &b) {
	std::vector < T > r(a.size());
	for (int i = 0; i < a.size(); i++) r[i] = a[i]*b;
	return r;
}

// iloczyn skalarny  2 wektorow
template < typename T > 
T operator *(const std::vector < T > &a, const std::vector < T > &b) {
	T r;
	for (int i = 0; i < a.size(); i++) r += a[i]*b;
	return r;
}

// dlugosc wektora a
template < typename T > 
T operator ~(const std::vector < T > &a) {
	T r;
	for (int i = 0; i < a.size(); i++) r += a[i]*a[i];
	return sqrt(r);
}

// za http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
template < typename T > 
T segPointDistance(const std::vector < T > &v, const std::vector < T > &w, const std::vector < T > &p) {
  // Return minimum distance between line segment vw and point p
  T l2;auto ld = v-w;
  for (int i = 0; i < ld.size(); i++) l2 += ld[i]*ld[i];
  if (l2 == 0.0) return ~(ld);   // v == w case
  // Consider the line extending the segment, parameterized as v + t (w - v).
  // We find projection of point p onto the line. 
  // It falls where t = [(p-v) . (w-v)] / |w-v|^2
  // We clamp t from [0,1] to handle points outside the segment vw.
  const float t = std::max(0.0, std::min(1.0, ((p - v)* (w - v)) / l2));
  const auto projection = v + (w - v)*t;  // Projection falls on the segment
  return ~(p-projection);
}


template < typename T > 
std::vector < T > intersect(const std::vector < T > &p1,
        const std::vector < T > &p2,
        const std::vector < T > &p3,
        const std::vector < T > &p4) {
double x1 = p1[0], x2 = p2[0], x3 = p3[0], x4 = p4[0];
double y1 = p1[1], y2 = p2[1], y3 = p3[1], y4 = p4[1];
 
double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
if (d == 0) return {};//return {p1[0], p1[1]};
 
double pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
double x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
double y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;

return {x,y};
}

template < typename T > 
std::vector < T > intersectSegments(const std::vector < T > &p1,
        const std::vector < T > &p2,
        const std::vector < T > &p3,
        const std::vector < T > &p4) {
double x1 = p1[0], x2 = p2[0], x3 = p3[0], x4 = p4[0];
double y1 = p1[1], y2 = p2[1], y3 = p3[1], y4 = p4[1];
 
double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
if (d == 0) return {};
 
double pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
double x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
double y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
 
// Check if the x and y coordinates are within both lines
if ( x < std::min(x1, x2) || x > std::max(x1, x2) ||
     x < std::min(x3, x4) || x > std::max(x3, x4) ) return {};
if ( y < std::min(y1, y2) || y > std::max(y1, y2) ||
     y < std::min(y3, y4) || y > std::max(y3, y4) ) return {};
return {x,y};
}

#endif
