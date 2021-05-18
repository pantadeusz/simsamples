#ifndef ___VECTORS_TP_PJATK_HPP___
#define ___VECTORS_TP_PJATK_HPP___

#include <cmath>

namespace tp::operators {

template<typename T1,typename T2>
inline T1 operator+(const T1& a, const T2& b) { 
    T1 ret;
    for (unsigned i = 0; i < ((a.size()>b.size())?b.size():a.size());i++) ret[i] = a[i] + b[i];
    return ret;
}

template<typename T1,typename T2>
inline T1 operator-(const T1& a, const T2& b) { 
    T1 ret;
    for (unsigned i = 0; i < ((a.size()>b.size())?b.size():a.size());i++) ret[i] = a[i] - b[i];
    return ret;
}

template<typename T>
inline double length(const T& a) { 
    double ret = 0.0;
    for (unsigned i = 0; i < a.size();i++) ret+= a[i] * a[i];
    return (ret == 0)?0.0:std::sqrt(ret);
}

}; // namespace tp


#endif
