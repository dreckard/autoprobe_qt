/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef _3D_MATH_H
#define _3D_MATH_H

#include <string>
#include <limits>
#include <cmath>

/*class Matrix4x4
{
public:
    float   m1, m5, m9,  m13,
            m2, m6, m10, m14,
            m3, m7, m11, m15,
            m4, m8, m12, m16;

    Matrix4x4(  float i1, float i5, float i9,  float i13,
                float i2, float i6, float i10, float i14,
                float i3, float i7, float i11, float i15,
                float i4, float i8, float i12, float i16 )
    {
        m1 = i1; m5 = i5; m9 = i9; m13 = i13;
        m2 = i2; m6 = i6; m10 = i10; m14 = i14;
        m3 = i3; m7 = i7; m11 = i11; m15 = i15;
        m4 = i4; m8 = i8; m12 = i12; m16 = i16;
    }


};

const Matrix4x4 Ident_4x4( 1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1 );*/
using namespace std;

inline bool fltEq( float flA, float flB, float flEpsilon = numeric_limits<float>::epsilon()*5 )
{
    if ( fabs(flA-flB) < flEpsilon )
        return true;
    return false;
}

class Matrix3x3
{
public:
    float   m1, m4, m7,
            m2, m5, m8,
            m3, m6, m9;

    Matrix3x3(  float i1, float i4, float i7,
                float i2, float i5, float i8,
                float i3, float i6, float i9 )
    {
        m1 = i1; m4 = i4; m7 = i7;
        m2 = i2; m5 = i5; m8 = i8;
        m3 = i3; m6 = i6; m9 = i9;
    }


};

const Matrix3x3 Ident_3x3( 1, 0, 0,
                           0, 1, 0,
                           0, 0, 1 );

//NOTE: With MinGW/GCC 4.4 targeting Win32, FLT_EPSILON = 1.92093E-7
const float VEC3_MIN_LENGTH = (numeric_limits<float>::epsilon()*3); //This saqs
class Vec3
{
public:
    float x;
    float y;
    float z;

    Vec3() { x = 0.0f; y = 0.0f; z = 0.0f; }

    Vec3( float flX, float flY, float flZ )
    {
        x = flX;
        y = flY;
        z = flZ;
    }

    float operator[]( unsigned char idx )
    {
        if ( idx == 0 )
            return x;
        else if ( idx == 1 )
            return y;
        else if ( idx == 2 )
            return z;
        else
            return 0.0f;
    }
    bool operator==( const Vec3& vecOther ) const
    {
        return ( fltEq( x, vecOther.x ) && fltEq( y, vecOther.y ) && fltEq( z, vecOther.z ) );
    }
    Vec3 operator+( const Vec3& vecOther ) const
    {
        return Vec3( x+vecOther.x, y+vecOther.y, z+vecOther.z );
    }
    Vec3 operator-( const Vec3& vecOther ) const
    {
        return Vec3( x-vecOther.x, y-vecOther.y, z-vecOther.z );
    }
    Vec3 operator*( const float flScalar ) const
    {
        return Vec3( x*flScalar, y*flScalar, z*flScalar );
    }
    Vec3 operator/( const float flScalar ) const
    {
        return Vec3( x/flScalar, y/flScalar, z/flScalar );
    }
    Vec3& operator*=( const float flScalar )
    {
        x *= flScalar;
        y *= flScalar;
        z *= flScalar;
        return *this;
    }
    Vec3& operator/=( const float flScalar )
    {
        x /= flScalar;
        y /= flScalar;
        z /= flScalar;
        return *this;
    }

    Vec3& operator+=( const Vec3& vecOther )
    {
        x += vecOther.x;
        y += vecOther.y;
        z += vecOther.z;
        return *this;
    }
    Vec3& operator-=( const Vec3& vecOther )
    {
        x -= vecOther.x;
        y -= vecOther.y;
        z -= vecOther.z;
        return *this;
    }

    Vec3& operator*=( const Matrix3x3& matXForm )
    {
        float tX = (x*matXForm.m1) + (y*matXForm.m4) + (z*matXForm.m7);
        float tY = (x*matXForm.m2) + (y*matXForm.m5) + (z*matXForm.m8);
               z = (x*matXForm.m3) + (y*matXForm.m6) + (z*matXForm.m9);
        x = tX;
        y = tY;

        return *this;
    }

    Vec3& Cross( const Vec3& vecOther )
    {
        float tX = (y*vecOther.z-z*vecOther.y);
        float tY = (z*vecOther.x-x*vecOther.z);
               z = (x*vecOther.y-y*vecOther.x);

        x = tX;
        y = tY;

        return *this;
    }

    float Dot( const Vec3& vecOther ) const
    {
        return (x * vecOther.x) + (y * vecOther.y) + (z * vecOther.z);
    }

    float Length( void ) const
    {
        return sqrt(x*x+y*y+z*z);
    }

    float Dist( const Vec3& vecOther ) const
    {
        return (*this-vecOther).Length();
    }

    void NormalizeInPlace( void )
    {
        *this /= Length();
    }

    Vec3 Normalize( void ) const
    {
        return *this / Length();
    }

    bool IsZero( void ) const
    {
        return (Length() < VEC3_MIN_LENGTH);
    }
    bool ParallelTo( const Vec3& vecOther ) const
    {
        //Zero vectors are not considered parallel to anything
        if ( IsZero() || vecOther.IsZero() )
            return false;

        if ( Normalize() == vecOther.Normalize() || Normalize() == (vecOther.Normalize() * -1)  )
            return true;
        return false;
    }
};
const Vec3 Vec3_Zero( 0, 0, 0 );

class Line
{
public:
    Vec3 m_vecPoint;
    Vec3 m_vecDir;

    Line() { m_vecPoint = Vec3_Zero; m_vecDir = Vec3_Zero; }
    Line( const Vec3& vecPoint, const Vec3& vecDir )
    {
        m_vecPoint = vecPoint;
        m_vecDir = vecDir;
    }

};

class Plane
{
public:
    Vec3 m_vecPoint;
    Vec3 m_vecNormal;

    Plane() { m_vecPoint = Vec3_Zero; m_vecNormal = Vec3_Zero; }
    Plane( const Vec3& vecPoint, const Vec3& vecNormal )
    {
        m_vecPoint = vecPoint;
        m_vecNormal = vecNormal;
    }

    Vec3 FindIntersection( const Line& line ) const
    {
        if ( m_vecNormal.IsZero() )
            return Vec3_Zero;
        return line.m_vecPoint + line.m_vecDir * (((m_vecPoint - line.m_vecPoint).Dot(m_vecNormal))/(line.m_vecDir.Dot(m_vecNormal)));
    }
};

#endif //_3D_MATH_H
