/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef UTILITY_H
#define UTILITY_H

#include <sstream>

//Everyone and his brother defines min and max, hence the rd_
template <typename T>
inline T rd_max(T x, T y)
{
     return (x > y) ? x : y;
}
template <typename T>
inline T rd_min(T x, T y)
{
     return (x < y) ? x : y;
}
template <typename T>
inline T rd_bound(T x, T minVal, T maxVal)
{
    T tmp = rd_max( x, minVal );
    tmp = rd_min( tmp, maxVal );
    return tmp;
}
template <typename T>
inline T rd_swap( T& x, T& y )
{
    T tmp = x;
    x = y;
    y = tmp;
}

//A couple of convenience fns to save typing out the string stream junk every time
template <typename T>
inline std::string AsString( T var, std::streamsize prec = 3 )
{
    std::stringstream s;
    s.setf( std::ios::fixed, std::ios::floatfield );
    s.precision( prec );
    s << var;
    return s.str();
}

template <typename T>
inline void FromString( T& var, const std::string& str )
{
    std::stringstream s( str );
    s >> var;
}

#endif // UTILITY_H
