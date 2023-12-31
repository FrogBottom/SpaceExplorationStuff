// Because this is a header-only library, we can't use #pragma once.
// We need to close the include guard before the implementation section.
#ifndef _GMATH_H
#define _GMATH_H

/*
Hey, 2023 Frog here - this is the graphics math library I usually use, but
I hacked it to use doubles instead of floats, because I thought we might
actually need the extra precision.

This is more than a matter of just replacing all instances of "float" with
"double", since we also have to strip the "f" off of any literals.

What that means is that I have disabled the SSE intrinsics, and it will
not compile if you re-enable them.

You're welcome to use this library for other stuff, but you probably want to
go find the un-hacked version, or ask me about it, or better yet just use the
Handmade Math library, which is just like this, but better!
*/

/*
================================================================================

Graphics Math Library version 1.2.4 (public domain)

GMath is a single-header C++ library for vector, matrix, and quaternion math,
primarily for use in graphics applications. As a C++ only library, it uses
operator overloading to implement math operations, rather than C style
functions. The general concept, most of the SSE code, and some of the more
complex matrix/quaternion functions are borrowed from the Handmade Math library
(also public domain).

Tested with MSVC compiler v15.9.11.

================================================================================

USAGE

To use the library, you must place the following two lines:

#define GMATH_IMPLEMENTATION
#include "GMath.h"

in exactly one C++ file. You can then include this header file wherever needed.

There are several options which alter the definitions provided by the library,
and so require you comment or remove a line in this file:

By default, the library will use SSE intrinsics for some operations if they are
available, for a minor speed boost. If you would like to disable SSE intrinsics,
you must comment or remove the following line:
*/

// #define GMATH_USE_SSE

/*
If you would like the library types and functions to be in the global namespace
instead of the GMath namespace, you must comment or remove the following line:
*/

// #define GMATH_USE_NAMESPACE

/*
There are also several options which can be defined in the source file
containing the GMATH_IMPLEMENTATION definition. They must be defined before you
include the GMath.h header:

If you would like to avoid including <math.h>, you'll need to define the
following, substituting your own functions for my_x_function:

#define GMATH_EXP my_exp_function
#define GMATH_LOG my_log_function
#define GMATH_SIN my_sin_function
#define GMATH_COS my_cos_function
#define GMATH_TAN my_tan_function
#define GMATH_ACOS my_acos_function
#define GMATH_ATAN my_atan_function
#define GMATH_SQRT my_sqrt_function
#define GMATH_ATAN2 my_atan2_function
#define GMATH_IMPLEMENTATION
#include "GMath.h"

If you define all of these functions, then GMath will not include the <math.h>
header. This is useful if you'd like to avoid depending on the CRT library.

When creating a projection matrix, the default behavior for GMATH is to use the
range [-1..1] for depth. If you'd like to use the range [0..1], you must define
GMATH_DEPTH_ZERO_TO_ONE in the source file, like so:

#define GMATH_DEPTH_ZERO_TO_ONE
#define GMATH_IMPLEMENTATION
#include "GMath.h"

By default, projection and view matrix functions assume a left-handed coordinate
system. If you would instead like to use a right-handed system, you must define
GMATH_RIGHT_HANDED in the source file, like so:

#define GMATH_RIGHT_HANDED
#define GMATH_IMPLEMENTATION
#include "GMath.h"

================================================================================

LICENSE

This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.

================================================================================

CREDITS

Primary Author: FrogBottom :)

GMath includes parts of version 1.9.0 of the Handmade Math library, which you
can find at the following link:

https://github.com/HandmadeMath/Handmade-Math

Handmade Math Credits:
Primary Author: Zakary Strange (zak@strangedev.net && @strangezak)

Functionality: Matt Mascarenhas (@miblo_), Aleph, FieryDrake (@fierydrake),
Gingerbill (@TheGingerBill), Ben Visness (@bvisness),
Trinton Bullard (@Peliex_Dev), @AntonDan

Fixes: Jeroen van Rijn (@J_vanRijn), Kiljacken (@Kiljacken),
Insofaras (@insofaras), Daniel Gibson (@DanielGibson)

===============================================================================

STILL TO DO

- Add an IVec4 type which makes use of intrinsics the same way that Vec4 does.

- Add more helper functions, generally. Things like NearlyEqual and Distance
for vectors, more matrix helpers, etc. Maybe quaternion functions that don't
normalize, etc.

- Create an extension library for common game engine transform stuff.

- Add more math funcs (floor, ceil, round, etc).

================================================================================
*/

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#endif

#if defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
#endif

#ifdef GMATH_USE_SSE
#undef GMATH_USE_SSE // We will redefine this if SSE is supported.
#ifdef _MSC_VER
// MSVC supports SSE in amd64 mode or _M_IX86_FP >= 1 (2 means SSE2).
#if defined(_M_AMD64) || ( defined(_M_IX86_FP) && _M_IX86_FP >= 1 )
#define GMATH_USE_SSE 1
#endif // _M_AMD64 OR _M_IX86_FP >= 1
#else // If not MSVC, check if SSE is supported anyway.
#ifdef __SSE__
#define GMATH_USE_SSE 1
#endif // __SSE__
#endif // _MSC_VER
#endif // GMATH_USE_SSE

#ifdef GMATH_USE_SSE
#include <xmmintrin.h>
#endif

#if !defined(GMATH_SIN) || !defined(GMATH_COS) || !defined(GMATH_TAN) || \
!defined(GMATH_SQRT) || !defined(GMATH_EXP) || !defined(GMATH_LOG) ||    \
!defined(GMATH_ACOS) || !defined(GMATH_ATAN)|| !defined(GMATH_ATAN2)
#include <math.h>
#endif

#ifndef GMATH_SIN
#define GMATH_SIN sin
#endif

#ifndef GMATH_COS
#define GMATH_COS cos
#endif

#ifndef GMATH_TAN
#define GMATH_TAN tan
#endif

#ifndef GMATH_SQRT
#define GMATH_SQRT sqrt
#endif

#ifndef GMATH_EXP
#define GMATH_EXP exp
#endif

#ifndef GMATH_LOG
#define GMATH_LOG log
#endif

#ifndef GMATH_ACOS
#define GMATH_ACOS acos
#endif

#ifndef GMATH_ATAN
#define GMATH_ATAN atan
#endif

#ifndef GMATH_ATAN2
#define GMATH_ATAN2 atan2
#endif

#define GMATH_PI 3.14159265359
#define GMATH_HALF_PI 1.57079632679
#define GMATH_TWO_PI 6.28318530718
#define GMATH_E 2.71828182845

#define GMATH_MIN(a, b) (a) > (b) ? (b) : (a)
#define GMATH_MAX(a, b) (a) < (b) ? (b) : (a)
#define GMATH_ABS(a) ((a) > 0 ? (a) : -(a))
#define GMATH_MOD(a, b) ((a) % (b)) >= 0 ? ((a) % (b)) : (((a) % (b)) + (b))

#ifdef GMATH_USE_NAMESPACE
namespace GMath
{
#endif
    struct IVec2;
    struct IVec3;
    struct Vec2;
    struct Vec3;
    struct Vec4;
    struct Mat4;
    struct Quat;
    
	struct IVec2
    {
        union
        {
            struct {int x, y;};
            struct {int r, g;};
            int data[2];
        };
		
		inline IVec2() = default;
		inline explicit IVec2(int fill);
		inline IVec2(int x, int y);
        inline explicit IVec2(int data[2]);
		
		inline IVec2& operator++();
        inline IVec2& operator--();
        inline IVec2& operator+=(IVec2 vec);
        inline IVec2& operator-=(IVec2 vec);
		inline IVec2& operator*=(IVec2 vec);
		inline IVec2& operator*=(int scalar);
		inline IVec2& operator/=(IVec2 vec);
		inline IVec2& operator/=(int scalar);
		
		inline operator Vec2() const;
		
		inline int& operator[](int i);
        inline const int& operator[](int i) const;
		
        const static IVec2 Zero;
        const static IVec2 One;
        const static IVec2 UnitX;
        const static IVec2 UnitY;
    };
	
    inline IVec2 operator*(IVec2 a, IVec2 b);
	inline IVec2 operator*(IVec2 a, int b);
	inline IVec2 operator*(int a, IVec2 b);
    inline IVec2 operator/(IVec2 a, IVec2 b);
	inline IVec2 operator/(IVec2 a, int b);
	inline IVec2 operator/(int a, IVec2 b);
    inline IVec2 operator+(IVec2 a, IVec2 b);
    inline IVec2 operator-(IVec2 a, IVec2 b);
	inline IVec2 operator-(IVec2 a);
	
    inline bool operator==(IVec2 a, IVec2 b);
	inline bool operator!=(IVec2 a, IVec2 b);
    
	struct IVec3
    {
        union
        {
            struct {int x, y, z;};
            struct {int r, g, b;};
            struct {IVec2 xy; int ignored_z;};
            struct {int ignored_x; IVec2 yz;};
            struct {IVec2 rg; int ignored_b;};
            struct {int ignored_r; IVec2 gb;};
            int data[3];
        };
		
		inline IVec3() = default;
		inline explicit IVec3(int fill);
		inline IVec3(int x, int y, int z);
		inline IVec3(IVec2 xy, int z);
		inline IVec3(int x, IVec2 yz);
		inline explicit IVec3(int data[3]);
		
        inline IVec3& operator++();
        inline IVec3& operator--();
        inline IVec3& operator+=(IVec3 vec);
        inline IVec3& operator-=(IVec3 vec);
		inline IVec3& operator*=(IVec3 vec);
		inline IVec3& operator*=(int scalar);
		inline IVec3& operator/=(IVec3 vec);
		inline IVec3& operator/=(int scalar);
		
		inline operator Vec3() const;
		
        inline int& operator[](int i);
        inline const int& operator[](int i) const;
		
        const static IVec3 Zero;
        const static IVec3 One;
        const static IVec3 UnitX;
        const static IVec3 UnitY;
        const static IVec3 UnitZ;
    };
	
    inline IVec3 operator*(IVec3 a, IVec3 b);
	inline IVec3 operator*(IVec3 a, int b);
	inline IVec3 operator*(int a, IVec3 b);
    inline IVec3 operator/(IVec3 a, IVec3 b);
	inline IVec3 operator/(IVec3 a, int b);
	inline IVec3 operator/(int a, IVec3 b);
    inline IVec3 operator+(IVec3 a, IVec3 b);
    inline IVec3 operator-(IVec3 a, IVec3 b);
	inline IVec3 operator-(IVec3 a);
	
    inline bool operator==(IVec3 a, IVec3 b);
	inline bool operator!=(IVec3 a, IVec3 b);
    
	struct Vec2
    {
        union
        {
            struct {double x, y;};
            struct {double r, g;};
            struct {double u, v;};
            double data[2];
        };
		
		inline Vec2() = default;
		inline explicit Vec2(double fill);
		inline Vec2(double x, double y);
        inline explicit Vec2(double data[2]);
		
        inline Vec2& operator+=(Vec2 vec);
        inline Vec2& operator-=(Vec2 vec);
		inline Vec2& operator*=(Vec2 vec);
		inline Vec2& operator*=(double scalar);
		inline Vec2& operator/=(Vec2 vec);
		inline Vec2& operator/=(double scalar);
		
		inline operator IVec2() const;
		
		inline double& operator[](int i);
        inline const double& operator[](int i) const;
		
        const static Vec2 Zero;
        const static Vec2 One;
        const static Vec2 UnitX;
        const static Vec2 UnitY;
    };
	
    inline Vec2 operator*(Vec2 a, Vec2 b);
	inline Vec2 operator*(double a, Vec2 b);
	inline Vec2 operator*(Vec2 a, double b);
    inline Vec2 operator/(double a, Vec2 b);
	inline Vec2 operator/(Vec2 a, double b);
	inline Vec2 operator/(double a, Vec2 b);
    inline Vec2 operator+(Vec2 a, Vec2 b);
    inline Vec2 operator-(Vec2 a, Vec2 b);
	inline Vec2 operator-(Vec2 a);
	
    inline bool operator==(Vec2 a, Vec2 b);
	inline bool operator!=(Vec2 a, Vec2 b);
    
	struct Vec3
    {
        union
        {
            struct {double x, y, z;};
            struct {double r, g, b;};
            struct {double u, v, w;};
            struct {IVec2 xy; double ignored_z;};
            struct {double ignored_x; IVec2 yz;};
            struct {IVec2 rg; double ignored_b;};
            struct {double ignored_r; IVec2 gb;};
            struct {IVec2 uv; double ignored_w;};
            struct {double ignored_u; IVec2 uv;};
            double data[3];
        };
		
		inline Vec3() = default;
		inline explicit Vec3(double fill);
		inline Vec3(double x, double y, double z);
		inline Vec3(Vec2 xy, double z);
		inline Vec3(double x, Vec2 yz);
		inline explicit Vec3(double data[3]);
		
        inline Vec3& operator+=(Vec3 vec);
        inline Vec3& operator-=(Vec3 vec);
		inline Vec3& operator*=(Vec3 vec);
		inline Vec3& operator*=(double scalar);
		inline Vec3& operator/=(Vec3 vec);
		inline Vec3& operator/=(double scalar);
		
		inline operator IVec3() const;
		
        inline double& operator[](int i);
        inline const double& operator[](int i) const;
		
        const static Vec3 Zero;
        const static Vec3 One;
        const static Vec3 UnitX;
        const static Vec3 UnitY;
        const static Vec3 UnitZ;
		const static Vec3 Red;
        const static Vec3 Green;
        const static Vec3 Blue;
		const static Vec3 Yellow;
        const static Vec3 Cyan;
        const static Vec3 Magenta;
		const static Vec3 Black;
        const static Vec3 White;
    };
	
    inline Vec3 operator*(Vec3 a, Vec3 b);
	inline Vec3 operator*(Vec3 a, Vec3 b);
	inline Vec3 operator*(Vec3 a, double b);
    inline Vec3 operator/(double a, Vec3 b);
	inline Vec3 operator/(Vec3 a, double b);
	inline Vec3 operator/(double a, Vec3 b);
    inline Vec3 operator+(Vec3 a, Vec3 b);
    inline Vec3 operator-(Vec3 a, Vec3 b);
	inline Vec3 operator-(Vec3 a);
	
    inline bool operator==(Vec3 a, Vec3 b);
	inline bool operator!=(Vec3 a, Vec3 b);
    
    struct Vec4
    {
        union
        {
            double data[4];
			
			struct {union {Vec3 xyz; struct {double x, y, z;};}; double w;};
			struct {double x; union {Vec3 yzw; struct {double y, z, w;};};};
            struct {Vec2 xy; double ignored_z; double ignored_w;};
            struct {double ignored_x; Vec2 yz; double ignored_w;};
            struct {double ignored_x; double ignored_y; Vec2 zw;};
			
            struct {union {Vec3 rgb; struct {double r, g, b;};}; double a;};
			struct {double r; union {Vec3 gba; struct {double g, b, a;};};};
			struct {Vec2 rg; double ignored_b; double ignored_a;};
			struct {double ignored_r; Vec2 gb; double ignored_a;};
			struct {double ignored_r; double ignored_g; Vec2 ba;};
            
            // SSE type (four packed single precision floats).
#ifdef GMATH_USE_SSE
            __m128 sse;
#endif
        };
		
		inline Vec4() = default;
		inline explicit Vec4(double fill);
		inline Vec4(Vec3 xyz, double w);
		inline Vec4(double x, Vec3 yzw);
		inline Vec4(Vec2 xy, Vec2 zw);
		inline Vec4(Vec2 xy, double z, double w);
		inline Vec4(double x, Vec2 yz, double w);
		inline Vec4(double x, double y, Vec2 zw);
		inline Vec4(double x, double y, double z, double w);
        inline explicit Vec4(double data[4]);
		inline explicit Vec4(Quat quat);
        
		inline Vec4& operator+=(Vec4 vec);
        inline Vec4& operator-=(Vec4 vec);
        inline Vec4& operator*=(Vec4 vec);
		inline Vec4& operator*=(double scalar);
        inline Vec4& operator/=(Vec4 vec);
        inline Vec4& operator/=(double scalar);
		
		inline double& operator[](int i);
        inline const double& operator[](int i) const;
        inline Vec4 operator-() const;
		
        const static Vec4 Zero;
        const static Vec4 One;
        const static Vec4 UnitX;
        const static Vec4 UnitY;
        const static Vec4 UnitZ;
        const static Vec4 UnitW;
        const static Vec4 Red;
        const static Vec4 Green;
        const static Vec4 Blue;
        const static Vec4 Yellow;
        const static Vec4 Cyan;
        const static Vec4 Magenta;
        const static Vec4 Black;
        const static Vec4 White;
    };
	
    inline Vec4 operator*(Vec4 a, Vec4 b);
	inline Vec4 operator*(Vec4 a, double b);
	inline Vec4 operator*(double a, Vec4 b);
    inline Vec4 operator/(Vec4 a, Vec4 b);
	inline Vec4 operator/(Vec4 a, double b);
	inline Vec4 operator/(double a, Vec4 b);
    inline Vec4 operator+(Vec4 a, Vec4 b);
    inline Vec4 operator-(Vec4 a, Vec4 b);
	inline Vec4 operator-(Vec4 a);
	
    inline bool operator==(Vec4 a, Vec4 b);
	inline bool operator!=(Vec4 a, Vec4 b);
    
    struct Mat4
    {
        union
        {
            Vec4 cols[4];
            double data[16];
			
#ifdef GMATH_USE_SSE
            __m128 sse[4];
#endif
        };
		
		inline Mat4() = default;
		inline Mat4(double diagonal);
		inline Mat4(Vec4 a, Vec4 b, Vec4 c, Vec4 d);
		inline Mat4(double data[16]);
		inline Mat4(Quat quat);
		
		inline Mat4& operator+=(Mat4 mat);
        inline Mat4& operator-=(Mat4 mat);
        inline Mat4& operator*=(Mat4 mat);
		
        inline Vec4& operator[](int i);
        inline const Vec4& operator[](int i) const;
		
        const static Mat4 Zero;
        const static Mat4 Identity;
    };
	
	inline Mat4 operator+(Mat4 a, Mat4 b);
    inline Mat4 operator-(Mat4 a, Mat4 b);
    inline Mat4 operator*(Mat4 a, Mat4 b);
    inline Vec4 operator*(Mat4 a, Vec4 b);
    
    struct Quat
    {
        union
        {
            struct {union {Vec3 xyz; struct {double x, y, z;};}; double w;};
            double data[4];
			//Vec4 vector;
#ifdef GMATH_USE_SSE
            __m128 sse;
#endif
        };
		
		inline Quat() = default;
		inline explicit Quat(double fill);
		inline Quat(Vec3 axis, double angle);
		inline explicit Quat(Vec4 values);
		inline Quat(double x, double y, double z, double w);
		inline Quat(double data[4]);
		inline Quat(Mat4 mat);
		inline Quat(Vec3 x_axis, Vec3 y_axis, Vec3 z_axis);
		inline Quat& operator*=(Quat b);
		inline double& operator[](int i);
        inline const double& operator[](int i) const;
		
        const static Quat Zero;
        const static Quat Identity;
    };
	
    inline Quat operator*(Quat a, Quat b);
    
	// Math function wrappers (inline, but enable compiler optimization if you
    // care about these being fast).
    inline double Sin(double radians);
    inline double Cos(double radians);
    inline double Tan(double radians);
    inline double ACos(double cos);
    inline double ATan(double tan);
    inline double Exp(double val);
    inline double Log(double val);
    inline double ATan2(double y, double x);
    inline double Pow(double val, double exp);
    inline double Sqrt(double val);
    inline double RSqrt(double val);
    inline double Radians(double degrees);
    inline double Degrees(double radians);
    inline double Min(double a, double b);
    inline double Max(double a, double b);
    inline double Abs(double val);
    inline double Pow(double base, int exponent);
	inline int Min(int a, int b);
    inline int Max(int a, int b);
    inline int Abs(int val);
    inline int Mod(int a, int b);
    
    // Utility functions.
    inline double Clamp(double val, double min, double max);
    inline double Lerp(double a, double b, double alpha);
    inline int Dot(IVec2 a, IVec2 b);
    inline int Dot(IVec3 a, IVec3 b);
    inline double Dot(Vec2 a, Vec2 b);
    inline double Dot(Vec3 a, Vec3 b);
    inline double Dot(Vec4 a, Vec4 b);
    inline Vec3 Cross(Vec3 a, Vec3 b);
    inline int LengthSquared(IVec2 vec);
    inline int LengthSquared(IVec3 vec);
    inline double LengthSquared(Vec2 vec);
    inline double LengthSquared(Vec3 vec);
    inline double LengthSquared(Vec4 vec);
    inline double Length(Vec2 vec);
    inline double Length(Vec3 vec);
    inline double Length(Vec4 vec);
    
    // Normalize returns a zero vector in case of a divide-by-zero.
    // FastNormalize uses an inverse square root, and does no divide-by-zero check.
    // SafeNormalize returns a zero vector if length < tolerance.
    inline Vec2 Normalize(Vec2 vec);
    inline Vec2 FastNormalize(Vec2 vec);
    inline Vec2 SafeNormalize(Vec2 vec, double tolerance = 0.001);
    inline Vec3 Normalize(Vec3 vec);
    inline Vec3 FastNormalize(Vec3 vec);
    inline Vec3 SafeNormalize(Vec3 vec, double tolerance = 0.001);
    inline Vec4 Normalize(Vec4 vec);
    inline Vec4 FastNormalize(Vec4 vec);
    inline Vec4 SafeNormalize(Vec4 vec, double tolerance = 0.001);
    
    inline Vec2 ClampLength(Vec2 vec, double min, double max);
    inline Vec3 ClampLength(Vec3 vec, double min, double max);
    inline Vec4 ClampLength(Vec4 vec, double min, double max);
    
    // Matrix functions.
    inline Mat4 Transpose(Mat4 mat);
    inline Mat4 CreatePerspectiveMatrix(double fov, double aspect, double near, double far);
    inline Mat4 CreateOrthoMatrix(double width, double height, double depth, double near_clip);
    inline Mat4 CreateOrthoMatrix(Vec3 extent, double near_clip);
    inline Mat4 CreateTranslationMatrix(Vec3 translation);
    inline Mat4 CreateRotationMatrix(Vec3 axis, double angle);
    inline Mat4 CreateScalingMatrix(Vec3 scale);
    inline Mat4 CreateLookAtMatrix(Vec3 eye_location, Vec3 target, Vec3 world_up);
    
    // Quaternion functions.
    inline double Dot(Quat a, Quat b);
    inline Quat Normalize(Quat quat);
    inline Quat Lerp(Quat a, Quat b, double alpha);
    inline Quat Slerp(Quat a, Quat b, double alpha);
    inline Quat Invert(Quat quat);
    
#ifdef GMATH_USE_NAMESPACE
};
#endif
#endif // GMATH_H

#ifdef GMATH_IMPLEMENTATION
#ifdef GMATH_USE_NAMESPACE
namespace GMath
{
#endif
    // Math function definitions.
	// TODO(Frog): These cause multiple include errors, move into header guard.
    double Sin(double radians) {return GMATH_SIN(radians);}
    double Cos(double radians) {return GMATH_COS(radians);}
    double Tan(double radians) {return GMATH_TAN(radians);}
    double ACos(double cos) {return GMATH_ACOS(cos);}
    double ATan(double tan) {return GMATH_ATAN(tan);}
    double Exp(double val) {return GMATH_EXP(val);}
    double Log(double val) {return GMATH_LOG(val);}
    double ATan2(double y, double x) {return GMATH_ATAN2(y, x);}
    
    double Pow(double val, double exponent)
    {
        return GMATH_EXP(exponent * GMATH_LOG(val));
    }
    
    double Sqrt(double val)
    {
#ifdef GMATH_USE_SSE
        __m128 in = _mm_set_ss(val);
        __m128 out = _mm_sqrt_ss(in);
        return _mm_cvtss_f32(out);
#else
        return GMATH_SQRT(val);
#endif
    }
    
    double RSqrt(double val)
    {
#ifdef GMATH_USE_SSE
        __m128 in = _mm_set_ss(val);
        __m128 out = _mm_rsqrt_ss(in);
        return _mm_cvtss_f32(out);
#else
        return 1.0 / GMATH_SQRT(val);
#endif
    }
    
    double Radians(double degrees)
    {
        return degrees * (GMATH_PI / 180.0);
    }
    
    double Degrees(double radians)
    {
        return (radians * 180.0) / GMATH_PI;
    }
    
    double Min(double a, double b)
    {
        return GMATH_MIN(a, b);
    }
    
    double Max(double a, double b)
    {
        return GMATH_MAX(a, b);
    }
    
    double Abs(double val)
    {
        return GMATH_ABS(val);
    }
    
	double Pow(double base, int exponent)
    {
        double result = 1.0;
        double mul = (exponent < 0) ? 1.0 / base : base;
        int x = (exponent < 0) ? -exponent : exponent;
        while (x)
        {
            if (x & 1)
            {
                result *= mul;
            }
            mul *= mul;
            x >>= 1;
        }
        return result;
    }
	
	int Min(int a, int b)
    {
        return GMATH_MIN(a, b);
    }
    
    int Max(int a, int b)
    {
        return GMATH_MAX(a, b);
    }
    
    int Abs(int val)
    {
        return GMATH_ABS(val);
    }
	
    int Mod(int a, int b)
    {
        return GMATH_MOD(a, b);
    }
    
    double Clamp(double val, double min, double max)
    {
        return GMATH_MAX(min, GMATH_MIN(max, val));
    }
    
    double Lerp(double a, double b, double alpha)
    {
        double clamped_alpha = Clamp(alpha, 0.0, 1.0);
        return (1.0 - clamped_alpha) * a + clamped_alpha * b;
    }
    
    int Dot(IVec2 a, IVec2 b) {return a.x * b.x + a.y * b.y;}
    int Dot(IVec3 a, IVec3 b) {return a.x * b.x + a.y + b.y + a.z * b.z;}
    double Dot(Vec2 a, Vec2 b) {return a.x * b.x + a.y * b.y;}
    double Dot(Vec3 a, Vec3 b) {return a.x * b.x + a.y * b.y + a.z * b.z;}
    double Dot(Vec4 a, Vec4 b)
    {
        double result;
#ifdef GMATH_USE_SSE
        __m128 result_one = _mm_mul_ps(a.sse, b.sse);
        __m128 result_two = _mm_shuffle_ps(result_one, result_two, _MM_SHUFFLE(2, 3, 0, 1));
        result_one = _mm_add_ps(result_one, result_two);
        result_two = _mm_shuffle_ps(result_one, result_one, _MM_SHUFFLE(0, 1, 2, 3));
        result_one = _mm_add_ps(result_one, result_two);
        _mm_store_ss(&result, result_one);
#else
        result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
#endif
        return result;
    }
    
    Vec3 Cross(Vec3 a, Vec3 b)
    {
        return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }
    
    int LengthSquared(IVec2 vec)
    {
        return Dot(vec, vec);
    }
    
    int LengthSquared(IVec3 vec)
    {
        return Dot(vec, vec);
    }
    
    double LengthSquared(Vec2 vec)
    {
        return Dot(vec, vec);
    }
    
    double LengthSquared(Vec3 vec)
    {
        return Dot(vec, vec);
    }
    
    double LengthSquared(Vec4 vec)
    {
        return Dot(vec, vec);
    }
    
    double Length(Vec2 vec)
    {
        return Sqrt(LengthSquared(vec));
    }
    
    double Length(Vec3 vec)
    {
        return Sqrt(LengthSquared(vec));
    }
    
    double Length(Vec4 vec)
    {
        return Sqrt(LengthSquared(vec));
    }
    
    Vec2 Normalize(Vec2 vec)
    {
        double length = Length(vec);
        return (length == 0.0) ? Vec2::Zero : vec / length;
    }
    
    Vec2 FastNormalize(Vec2 vec)
    {
        return vec * RSqrt(Dot(vec, vec));
    }
    
    Vec2 SafeNormalize(Vec2 vec, double tolerance)
    {
        double length = Length(vec);
        return (length < tolerance) ? Vec2::Zero : vec / length;
    }
    
    Vec3 Normalize(Vec3 vec)
    {
        double length = Length(vec);
        return (length == 0.0) ? Vec3::Zero : vec / length;
    }
    
    Vec3 FastNormalize(Vec3 vec)
    {
        return vec * RSqrt(Dot(vec, vec));
    }
    
    Vec3 SafeNormalize(Vec3 vec, double tolerance)
    {
        double length = Length(vec);
        return (length < tolerance) ? Vec3::Zero : vec / length;
    }
    
    
    Vec4 Normalize(Vec4 vec)
    {
        double length = Length(vec);
        return (length == 0.0) ? Vec4::Zero : vec / Vec4(length);
    }
    
    Vec4 FastNormalize(Vec4 vec)
    {
        return vec * Vec4(RSqrt(Dot(vec, vec)));
    }
    
    Vec4 SafeNormalize(Vec4 vec, double tolerance)
    {
        double length = Length(vec);
        return (length < tolerance) ? Vec4::Zero : vec / Vec4(length);
    }
    
    Vec2 ClampLength(Vec2 vec, double min, double max)
    {
        double length = Length(vec);
        if (length < min || length > max) vec = Normalize(vec) * Clamp(length, min, max);
        return vec;
    }
    
    Vec3 ClampLength(Vec3 vec, double min, double max)
    {
        double length = Length(vec);
        if (length < min || length > max) vec = Normalize(vec) * Clamp(length, min, max);
        return vec;
    }
    
    Vec4 ClampLength(Vec4 vec, double min, double max)
    {
        double length = Length(vec);
        if (length < min || length > max) vec = Normalize(vec) * Vec4(Clamp(length, min, max));
        return vec;
    }
    
    // Matrix math.
    
    Mat4 Transpose(Mat4 mat)
    {
#ifdef GMATH_USE_SSE
        _MM_TRANSPOSE4_PS(mat.sse[0], mat.sse[1], mat.sse[2], mat.sse[3]);
#else
        Vec4 row1 = {mat.cols[0].x, mat.cols[1].x, mat.cols[2].x, mat.cols[3].x};
        Vec4 row2 = {mat.cols[0].y, mat.cols[1].y, mat.cols[2].y, mat.cols[3].y};
        Vec4 row3 = {mat.cols[0].z, mat.cols[1].z, mat.cols[2].z, mat.cols[3].z};
        Vec4 row4 = {mat.cols[0].w, mat.cols[1].w, mat.cols[2].w, mat.cols[3].w};
        mat[0] = row1;
        mat[1] = row2;
        mat[3] = row3;
        mat[4] = row4;
#endif
        return mat;
    }
    
    Mat4 CreateOrthoMatrix(double width, double height, double near, double far)
    {
        Mat4 result = {};
        result[0][0] = 2.0 / width;
        result[1][1] = 2.0 / height;
#ifdef GMATH_DEPTH_ZERO_TO_ONE
        double numerator = 1.0;
#else
        double numerator = 2.0;
#endif
#ifdef GMATH_RIGHT_HANDED
        result[2][2] = numerator / (near - far);
		result[2][3] = near / (near - far);
#else
        result[2][2] = numerator / (far - near);
		result[2][3] = -near / (far - near);
#endif
        result[3][3] = 1.0;
        return result;
    }
    
    Mat4 CreatePerspectiveMatrix(double fov, double aspect, double n, double f)
    {
        Mat4 result = {};
        double cotan = 1.0 / Tan(fov * (GMATH_PI / 360.0));
        result[0][0] = cotan / aspect;
        result[1][1] = -cotan;
#ifdef GMATH_DEPTH_ZERO_TO_ONE
        double numerator = n * f;
#else
        double numerator = n * f * 2.0;
#endif
#ifdef GMATH_RIGHT_HANDED
        result[2][2] = f / (n - f);
        result[2][3] = -1.0;
        result[3][2] = numerator / (n - f);
#else
        result[2][2] = f / (f - n);
        result[2][3] = 1.0;
        result[3][2] = -numerator / (f - n);
#endif
        return result;
    }
    
    Mat4 CreateTranslationMatrix(Vec3 translation)
    {
        Mat4 result = Mat4(1.0);
        result[3] = {translation, 1.0};
        return result;
    }
    
    
    Mat4 CreateScalingMatrix(Vec3 scale)
    {
        Mat4 result = {};
        result[0][0] = scale.x;
        result[1][1] = scale.y;
        result[2][2] = scale.z;
        result[3][3] = 1.0;
        return result;
    }
    
    Mat4 CreateRotationMatrix(Vec3 axis, double angle)
    {
        Mat4 result = {};
        axis = Normalize(axis);
        
        double sin = Sin(Radians(angle));
        double cos = Cos(Radians(angle));
        double cos_value = 1.0 - cos;
        result[0][0] = (axis.x * axis.x * cos_value) + cos;
        result[0][1] = (axis.x * axis.y * cos_value) + (axis.z * sin);
        result[0][2] = (axis.x * axis.z * cos_value) - (axis.y * sin);
        
        result[1][0] = (axis.y * axis.x * cos_value) - (axis.z * sin);;
        result[1][1] = (axis.y * axis.y * cos_value) + cos;
        result[1][2] = (axis.y * axis.z * cos_value) + (axis.x - sin);
        
        result[2][0] = (axis.z * axis.x * cos_value) + (axis.y * sin);
        result[2][1] = (axis.z * axis.y * cos_value) - (axis.x * sin);
        result[2][2] = (axis.z * axis.z * cos_value) + cos;
        result[3][3] = 1.0;
        return result;
    }
    
    Mat4 CreateLookAtMatrix(Vec3 location, Vec3 target, Vec3 world_up)
    {
        Mat4 result;
#ifdef GMATH_RIGHT_HANDED
        Vec3 forward = Normalize(location - target);
        Vec3 axis = -forward;
#else
        Vec3 forward = Normalize(target - location);
        Vec3 axis = forward;
#endif
        Vec3 right = Normalize(Cross(world_up, forward));
        Vec3 up = Cross(forward, right);
        result[0] = {right.x, up.x, axis.x, 0.0};
        result[1] = {right.y, up.y, axis.y, 0.0};
        result[2] = {right.z, up.z, axis.z, 0.0};
        result[3] = {-Dot(right, location), -Dot(up, location), -Dot(forward, location), 1.0};
        return result;
    }
    
    double Dot(Quat a, Quat b)
    {
        double result;
#ifdef GMATH_USE_SSE
        __m128 result_one = _mm_mul_ps(a.sse, b.sse);
        __m128 result_two = _mm_shuffle_ps(result_one, result_one, _MM_SHUFFLE(2, 3, 0, 1));
        result_one = _mm_add_ps(result_one, result_two);
        result_two = _mm_shuffle_ps(result_one, result_one, _MM_SHUFFLE(0, 1, 2, 3));
        result_one = _mm_add_ps(result_one, result_two);
        _mm_store_ss(&result, result_one);
#else
        result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
#endif
        return result;
    }
    
    Quat Normalize(Quat quat)
    {
        double length = Sqrt(Dot(quat, quat));
		if (length == 0.0) return Quat::Zero; // TODO(Frog): We should be using an epsilon here.
		else return Quat(Vec4(quat) / length);
		//else return Quat(quat.vector / Vec4(length));
    }
    
    Quat Lerp(Quat a, Quat b, double alpha)
    {
        Quat result;
#ifdef GMATH_USE_SSE
        double clamped_alpha = Clamp(alpha, 0.0, 1.0);
        __m128 a_scalar = _mm_set1_ps(1.0 - clamped_alpha);
        __m128 b_scalar = _mm_set1_ps(clamped_alpha);
        __m128 result_one = _mm_mul_ps(a.sse, a_scalar);
        __m128 result_two = _mm_mul_ps(b.sse, b_scalar);
        result.sse = _mm_add_ps(result_one, result_two);
#else
        result.x = Lerp(a.x, b.x, alpha);
        result.y = Lerp(a.y, b.y, alpha);
        result.z = Lerp(a.z, b.z, alpha);
        result.w = Lerp(a.w, b.w, alpha);
#endif
        return Normalize(result);
    }
    
    Quat Slerp(Quat a, Quat b, double alpha)
    {
        double clamped_alpha = Clamp(alpha, 0.0, 1.0);
        double angle = ACos(Dot(a, b));
		Vec4 left = Vec4(a) * Sin((1.0 - clamped_alpha) * angle);
		Vec4 right = Vec4(b) * Sin(clamped_alpha * angle);
        return Quat((left + right) * (1.0 / Sin(angle)));
    }
    
    Quat Invert(Quat quat)
    {
		return Quat(Vec4(-quat.x, -quat.y, -quat.z, quat.w) / Dot(quat, quat));
    }
    
	// IVec2 Implementation
	// ============================================================================
	
	IVec2::IVec2(int fill) : x(fill), y(fill) {}
	IVec2::IVec2(int x, int y) : x(x), y(y) {}
	IVec2::IVec2(int data[2]) : data{data[0], data[1]} {}
	
	IVec2& IVec2::operator+=(IVec2 o) {x += o.x;y += o.y;return *this;}
	IVec2& IVec2::operator-=(IVec2 o) {x -= o.x;y -= o.y;return *this;}
	IVec2& IVec2::operator*=(IVec2 o) {x *= o.x;y *= o.x;return *this;}
	IVec2& IVec2::operator*=(int o) {x *= o;y *= o;return *this;}
	IVec2& IVec2::operator/=(IVec2 o) {x /= o.x;y /= o.y;return *this;}
	IVec2& IVec2::operator/=(int o) {x /= o;y /= o;return *this;}
	
	IVec2 operator*(IVec2 a, IVec2 b) {return {a.x * b.x, a.y};}
	IVec2 operator*(IVec2 a, int b) {return {a.x * b, a.y * b};}
	IVec2 operator*(int a, IVec2 b) {return {b.x * a, b.y * a};}
	
	IVec2 operator/(IVec2 a, IVec2 b) {return {a.x / b.x, a.y / b.y};}
	IVec2 operator/(IVec2 a, int b) {return {a.x / b, a.y / b};}
	IVec2 operator/(int a, IVec2 b) {return {b.x / a, b.y / a};}
	
	IVec2 operator+(IVec2 a, IVec2 b) {return {a.x + b.x, a.y + b.y};}
	
	IVec2 operator-(IVec2 a, IVec2 b) {return {a.x - b.x, a.y - b.y};}
	IVec2 operator-(IVec2 a) {return {-a.x, -a.y};}
	
	IVec2::operator Vec2() const {return {(double)x, (double)y};}
	
	int& IVec2::operator[](int i) {return data[i];}
	const int& IVec2::operator[](int i) const {return data[i];}
	
	bool operator==(IVec2 a, IVec2 b) {return (a.x == b.x && a.y == b.y);}
	bool operator!=(IVec2 a, IVec2 b) {return (a.x != b.x && a.y != b.y);}
	
	const IVec2 IVec2::Zero = {0, 0};
	const IVec2 IVec2::One = {1, 1};
	const IVec2 IVec2::UnitX = {1, 0};
	const IVec2 IVec2::UnitY = {0, 1};
	
	// IVec3 Implementation
	// ============================================================================
	
	IVec3::IVec3(int fill) : x(fill), y(fill), z(fill) {}
	IVec3::IVec3(int x, int y, int z) : x(x), y(y), z(z) {}
	IVec3::IVec3(IVec2 xy, int z) : xy(xy), z(z) {}
	IVec3::IVec3(int x, IVec2 yz) : x(x), yz(yz) {}
	IVec3::IVec3(int data[3]) : data{data[0], data[1], data[2]} {}
	
	IVec3& IVec3::operator+=(IVec3 o) {x += o.x;y += o.y;z += o.z;return *this;}
	IVec3& IVec3::operator-=(IVec3 o) {x -= o.x;y -= o.y;z -= o.z;return *this;}
	IVec3& IVec3::operator*=(IVec3 o) {x *= o.x;y *= o.y;z *= o.z;return *this;}
	IVec3& IVec3::operator*=(int o) {x *= o;y *= o;z *= o;return *this;}
	IVec3& IVec3::operator/=(IVec3 o) {x /= o.x;y /= o.y;z /= o.z;return *this;}
	IVec3& IVec3::operator/=(int o) {x /= o;y /= o;z /= o;return *this;}
	
	IVec3 operator*(IVec3 a, IVec3 b) {return {a.x * b.x, a.y * b.y, a.z * b.z};}
	IVec3 operator*(IVec3 a, int b) {return {a.x * b, a.y * b, a.z * b};}
	IVec3 operator*(int a, IVec3 b) {return {a * b.x, a * b.y, a * b.z};}
	IVec3 operator/(IVec3 a, IVec3 b) {return {a.x / b.x, a.y / b.y, a.z / b.z};}
	IVec3 operator/(IVec3 a, int b) {return {a.x / b, a.y / b, a.z / b};}
	IVec3 operator/(int a, IVec3 b) {return {a / b.x, a / b.y, a / b.z};}
	IVec3 operator+(IVec3 a, IVec3 b) {return {a.x + b.x, a.y + b.y, a.z + b.z};}
	IVec3 operator-(IVec3 a, IVec3 b) {return {a.x - b.x, a.y - b.y, a.z - b.z};}
	IVec3 operator-(IVec3 a) {return {-a.x, -a.y, -a.z};}
	
	IVec3::operator Vec3() const {return {(double)x, (double)y, (double)z};}
	
	int& IVec3::operator[](int i) {return data[i];}
	const int& IVec3::operator[](int i) const {return data[i];}
	
	bool operator==(IVec3 a, IVec3 b) {return (a.x == b.x && a.y == b.y && a.z == b.z);}
	bool operator!=(IVec3 a, IVec3 b) {return (a.x != b.x && a.y != b.y && a.z != b.z);}
	
	const IVec3 IVec3::Zero = {0, 0, 0};
	const IVec3 IVec3::One = {1, 1, 1};
	const IVec3 IVec3::UnitX = {1, 0, 0};
	const IVec3 IVec3::UnitY = {0, 1, 0};
	const IVec3 IVec3::UnitZ = {0, 0, 1};
	
	// Vec2 Implementation
	// ============================================================================
	
	Vec2::Vec2(double fill) : x(fill), y(fill) {}
	Vec2::Vec2(double x, double y) : x(x), y(y) {}
	Vec2::Vec2(double data[2]) : data{data[0], data[1]} {}
	Vec2& Vec2::operator+=(Vec2 o) {x += o.x;y += o.y;return *this;}
	Vec2& Vec2::operator-=(Vec2 o) {x -= o.x;y -= o.y;return *this;}
	Vec2& Vec2::operator*=(Vec2 o) {x *= o.x;y *= o.x;return *this;}
	Vec2& Vec2::operator*=(double o) {x *= o;y *= o;return *this;}
	Vec2& Vec2::operator/=(Vec2 o) {x /= o.x;y /= o.y;return *this;}
	Vec2& Vec2::operator/=(double o) {x /= o;y /= o;return *this;}
	
	Vec2 operator*(Vec2 a, Vec2 b) {return {a.x * b.x, a.y * b.y};}
	Vec2 operator*(Vec2 a, double b) {return {a.x * b, a.y * b};}
	Vec2 operator*(double a, Vec2 b) {return {a * b.x, a * b.y};}
	Vec2 operator/(Vec2 a, Vec2 b) {return {a.x / b.x, a.y / b.y};}
	Vec2 operator/(Vec2 a, double b) {return {a.x / b, a.y / b};}
	Vec2 operator/(double a, Vec2 b) {return {a / b.x, a / b.y};}
	Vec2 operator+(Vec2 a, Vec2 b) {return {a.x + b.x, a.y + b.y};}
	Vec2 operator-(Vec2 a, Vec2 b) {return {a.x - b.x, a.y - b.y};}
	Vec2 operator-(Vec2 a) {return {-a.x, -a.y};}
	
	Vec2::operator IVec2() const {return {(int)x, (int)y};}
	
	double& Vec2::operator[](int i) {return data[i];}
	const double& Vec2::operator[](int i) const {return data[i];}
	
	bool operator==(Vec2 a, Vec2 b) {return (a.x == b.x && a.y == b.y);}
	bool operator!=(Vec2 a, Vec2 b) {return (a.x != b.x && a.y != b.y);}
	
	const Vec2 Vec2::Zero = {0, 0};
	const Vec2 Vec2::One = {1, 1};
	const Vec2 Vec2::UnitX = {1, 0};
	const Vec2 Vec2::UnitY = {0, 1};
	
	// Vec3 Implementation
	// ============================================================================
	
	Vec3::Vec3(double fill) : x(fill), y(fill), z(fill) {}
	Vec3::Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
	Vec3::Vec3(Vec2 xy, double z) : xy(xy), z(z) {}
	Vec3::Vec3(double x, Vec2 yz) : x(x), yz(yz) {}
	Vec3::Vec3(double data[3]) : data{data[0], data[1], data[2]} {}
	
	Vec3& Vec3::operator+=(Vec3 o) {x += o.x;y += o.y;z += o.z;return *this;}
	Vec3& Vec3::operator-=(Vec3 o) {x -= o.x;y -= o.y;z -= o.z;return *this;}
	Vec3& Vec3::operator*=(Vec3 o) {x *= o.x;y *= o.y;z *= o.z;return *this;}
	Vec3& Vec3::operator*=(double o) {x *= o;y *= o;z *= o;return *this;}
	Vec3& Vec3::operator/=(Vec3 o) {x /= o.x;y /= o.y;z /= o.z;return *this;}
	Vec3& Vec3::operator/=(double o) {x /= o;y /= o;z /= o;return *this;}
	
	Vec3 operator*(Vec3 a, Vec3 b) {return {a.x * b.x, a.y * b.y, a.z * b.z};}
	Vec3 operator*(Vec3 a, double b) {return {a.x * b, a.y * b, a.z * b};}
	Vec3 operator*(double a, Vec3 b) {return {a * b.x, a * b.y, a * b.z};}
	Vec3 operator/(Vec3 a, Vec3 b) {return {a.x / b.x, a.y / b.y, a.z / b.z};}
	Vec3 operator/(Vec3 a, double b) {return {a.x / b, a.y / b, a.z / b};}
	Vec3 operator/(double a, Vec3 b) {return {a / b.x, a / b.y, a / b.z};}
	Vec3 operator+(Vec3 a, Vec3 b) {return {a.x + b.x, a.y + b.y, a.z + b.z};}
	Vec3 operator-(Vec3 a, Vec3 b) {return {a.x - b.x, a.y - b.y, a.z - b.z};}
	Vec3 operator-(Vec3 a) {return {-a.x, -a.y, -a.z};}
	
	Vec3::operator IVec3() const {return {(int)x, (int)y, (int)z};}
	
	double& Vec3::operator[](int i) {return data[i];}
	const double& Vec3::operator[](int i) const {return data[i];}
	
	bool operator==(Vec3 a, Vec3 b) {return (a.x == b.x && a.y == b.y && a.z == b.z);}
	bool operator!=(Vec3 a, Vec3 b) {return (a.x != b.x && a.y != b.y && a.z != b.z);}
	
	const Vec3 Vec3::Zero = {0, 0, 0};
	const Vec3 Vec3::One = {1, 1, 1};
	const Vec3 Vec3::UnitX = {1, 0, 0};
	const Vec3 Vec3::UnitY = {0, 1, 0};
	const Vec3 Vec3::UnitZ = {0, 0, 1};
	const Vec3 Vec3::Red = {1, 0, 0};
	const Vec3 Vec3::Green = {0, 1, 0};
	const Vec3 Vec3::Blue = {0, 0, 1};
	const Vec3 Vec3::Yellow = {1, 1, 0};
	const Vec3 Vec3::Cyan = {0, 1, 1};
	const Vec3 Vec3::Magenta = {1, 0, 1};
	const Vec3 Vec3::Black = {0, 0, 0};
	const Vec3 Vec3::White = {1, 1, 1};
	
	// Vec4 Implementation
	// ============================================================================
	
#ifdef GMATH_USE_SSE
	Vec4::Vec4(double fill) : sse(_mm_set1_ps(fill)) {}
	Vec4::Vec4(Vec3 xyz, double w) : sse(_mm_setr_ps(xyz.x, xyz.y, xyz.z, w)) {}
	Vec4::Vec4(double x, Vec3 yzw) : sse(_mm_setr_ps(x, yzw.x, yzw.y, yzw.z)) {}
	Vec4::Vec4(Vec2 xy, Vec2 zw) : sse(_mm_setr_ps(xy.x, xy.y, zw.x, zw.y)) {}
	Vec4::Vec4(Vec2 xy, double z, double w) : sse(_mm_setr_ps(xy.x, xy.y, z, w)) {}
	Vec4::Vec4(double x, Vec2 yz, double w) : sse(_mm_setr_ps(x, yz.x, yz.y, w)) {}
	Vec4::Vec4(double x, double y, Vec2 zw) : sse(_mm_setr_ps(x, y, zw.x, zw.y)) {}
	Vec4::Vec4(double x, double y, double z, double w) : sse(_mm_setr_ps(x, y, z, w)) {}
	Vec4::Vec4(double data[4]) : sse(_mm_setr_ps(data[0], data[1], data[2], data[3])) {}
	Vec4::Vec4(Quat quat) : sse(quat.sse) {}
	
	Vec4& Vec4::operator+=(Vec4 o) {sse = _mm_add_ps(sse, o.sse);return *this;}
	Vec4& Vec4::operator-=(Vec4 o) {sse = _mm_sub_ps(sse, o.sse);return *this;}
	Vec4& Vec4::operator*=(Vec4 o) {sse = _mm_mul_ps(sse, o.sse);return *this;}
	Vec4& Vec4::operator*=(double o) {__m128 c = _mm_set1_ps(o);sse = _mm_mul_ps(sse, c);return *this;}
	Vec4& Vec4::operator/=(Vec4 o) {sse = _mm_div_ps(sse, o.sse);return *this;}
	Vec4& Vec4::operator/=(double o) {__m128 c = _mm_set1_ps(o);sse = _mm_div_ps(sse, c);return *this;}
	
	Vec4 operator*(Vec4 a, Vec4 b) {Vec4 o;o.sse = _mm_mul_ps(a.sse, b.sse);return o;}
	Vec4 operator*(Vec4 a, double b) {Vec4 o;__m128 c = _mm_set1_ps(b);o.sse = _mm_mul_ps(a.sse, c);return o;}
	Vec4 operator*(double a, Vec4 b) {Vec4 o;__m128 c = _mm_set1_ps(a);o.sse = _mm_mul_ps(c, b.sse);return o;}
	Vec4 operator/(Vec4 a, Vec4 b) {Vec4 o;o.sse = _mm_div_ps(a.sse, b.sse);return o;}
	Vec4 operator/(Vec4 a, double b) {Vec4 o;__m128 c = _mm_set1_ps(b);o.sse = _mm_div_ps(a.sse, c);return o;}
	Vec4 operator/(double a, Vec4 b) {Vec4 o;__m128 c = _mm_set1_ps(a);o.sse = _mm_div_ps(c, b.sse);return o;}
	Vec4 operator+(Vec4 a, Vec4 b) {Vec4 o;o.sse = _mm_add_ps(a.sse, b.sse);return o;}
	Vec4 operator-(Vec4 a, Vec4 b) {Vec4 o;o.sse = _mm_sub_ps(a.sse, b.sse);return o;}
	Vec4 operator-(Vec4 a) {Vec4 o;__m128 c = _mm_set1_ps(-1);o.sse = _mm_mul_ps(a.sse, c);return o;}
#else
	Vec4::Vec4(double fill) : x(fill), y(fill), z(fill), w(fill) {}
	Vec4::Vec4(Vec3 xyz, double w) : xyz(xyz), w(w) {}
	Vec4::Vec4(double x, Vec3 yzw) : x(x), yzw(yzw) {}
	Vec4::Vec4(Vec2 xy, Vec2 zw) : xy(xy), zw(zw) {}
	Vec4::Vec4(Vec2 xy, double z, double w) : xy(xy), z(z), w(w) {}
	Vec4::Vec4(double x, Vec2 yz, double w) : x(x), yz(yz), w(w) {}
	Vec4::Vec4(double x, double y, Vec2 zw) : x(x), y(y), zw(zw) {}
	Vec4::Vec4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
	Vec4::Vec4(double data[4]) : data{data[0], data[1], data[2], data[3]} {}
	Vec4::Vec4(Quat quat) : x(quat.x), y(quat.y), z(quat.z), w(quat.w) {}
	
	Vec4& Vec4::operator+=(Vec4 o) {x += o.x;y += o.y;z += o.z;w += o.w;return *this;}
	Vec4& Vec4::operator-=(Vec4 o) {x -= o.x;y -= o.y;z -= o.z;w -= o.w;return *this;}
	Vec4& Vec4::operator*=(Vec4 o) {x *= o.x;y *= o.y;z *= o.z;w *= o.w;return *this;}
	Vec4& Vec4::operator/=(Vec4 o) {x /= o.x;y /= o.y;z /= o.z;w /= o.w;return *this;}
	
	Vec4 operator*(Vec4 a, Vec4 b) {return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};}
    Vec4 operator*(Vec4 a, double b) {return {a.x * b, a.y * b, a.z * b, a.w * b};}
	Vec4 operator*(double a, Vec4 b) {return {a * b.x, a * b.y, a * b.z, a * b.w};}
	Vec4 operator/(Vec4 a, Vec4 b) {return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};}
    Vec4 operator/(Vec4 a, double b) {return {a.x / b, a.y / b, a.z / b, a.w / b};}
	Vec4 operator/(double a, Vec4 b) {return {a / b.x, a / b.y, a / b.z, a / b.w};}
	Vec4 operator+(Vec4 a, Vec4 b) {return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};}
	Vec4 operator-(Vec4 a, Vec4 b) {return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};}
	Vec4 operator-(Vec4 a) {return {-a.x, -a.y, -a.z, -a.w};}
#endif
	
	double& Vec4::operator[](int i) {return data[i];}
	const double& Vec4::operator[](int i) const {return data[i];}
	Vec4 Vec4::operator-() const {return {-x, -y, -z, -w};}
	
	bool operator==(Vec4 a, Vec4 b) {return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);}
	bool operator!=(Vec4 a, Vec4 b) {return (a.x != b.x && a.y != b.y && a.z != b.z && a.w != b.w);}
	
	const Vec4 Vec4::Zero = {0, 0, 0, 0};
	const Vec4 Vec4::One = {1, 1, 1, 1};
	const Vec4 Vec4::UnitX = {1, 0, 0, 0};
	const Vec4 Vec4::UnitY = {0, 1, 0, 0};
	const Vec4 Vec4::UnitZ = {0, 0, 1, 0};
	const Vec4 Vec4::UnitW = {0, 0, 0, 1};
	const Vec4 Vec4::Red = {1, 0, 0, 1};
	const Vec4 Vec4::Green = {0, 1, 0, 1};
	const Vec4 Vec4::Blue = {0, 0, 1, 1};
	const Vec4 Vec4::Yellow = {1, 1, 0, 1};
	const Vec4 Vec4::Cyan = {0, 1, 1, 1};
	const Vec4 Vec4::Magenta = {1, 0, 1, 1};
	const Vec4 Vec4::Black = {0, 0, 0, 1};
	const Vec4 Vec4::White = {1, 1, 1, 1};
	
	// Mat4 Implementation
	// ============================================================================
	
	Mat4::Mat4(double d) : cols{{d, 0, 0, 0}, {0, d, 0, 0}, {0, 0, d, 0}, {0, 0, 0, d}} {}
	Mat4::Mat4(Vec4 a, Vec4 b, Vec4 c, Vec4 d) : cols{a, b, c, d} {}
	Mat4::Mat4(double d[16]) :
	data{d[0], d[1], d[2], d[3],
		d[4], d[5], d[6], d[7],
		d[8], d[9], d[10], d[11],
		d[12], d[13], d[14], d[15]} {}
	
	Mat4::Mat4(Quat quat)
    {
        quat = Normalize(quat);
        double xx = quat.x * quat.x;
        double yy = quat.y * quat.y;
        double zz = quat.z * quat.z;
        double xy = quat.x * quat.y;
        double xz = quat.x * quat.z;
        double yz = quat.y * quat.z;
        double wx = quat.w * quat.x;
        double wy = quat.w * quat.y;
        double wz = quat.w * quat.z;
		cols[0] = {1.0 - 2.0 * (yy + zz), 2.0 * (xy + wz), 2.0 * (xz - wy), 0.0};
		cols[1] = {2.0 * (xy - wz), 1.0 - 2.0 * (xx + zz), 2.0 * (yz + wx), 0.0};
		cols[2] = {2.0 * (xz + wy), 2.0 * (yz - wx), 1.0 - 2.0 * (xx + yy), 0.0};
		cols[3] = {0.0, 0.0, 0.0, 1.0};
    }
    
	Mat4& Mat4::operator+=(Mat4 b) {cols[0] += b[0]; cols[1] += b[1]; cols[2] += b[2]; cols[3] += b[3];}
	Mat4& Mat4::operator-=(Mat4 b) {cols[0] -= b[0]; cols[1] -= b[1]; cols[2] -= b[2]; cols[3] -= b[3];}
	Mat4& Mat4::operator*=(Mat4 b) {*this = *this * b; return *this;}
	
	Mat4 operator+(Mat4 a, Mat4 b) {return {a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]};}
	Mat4 operator-(Mat4 a, Mat4 b) {return {a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]};}
#ifdef GMATH_USE_SSE
	static  __m128 LinearCombineSSE(__m128 left, Mat4 right)
	{
		__m128 result;
		result = _mm_mul_ps(_mm_shuffle_ps(left, left, 0x00), right.sse[0]);
		result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(left, left, 0x55), right.sse[1]));
		result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(left, left, 0xaa), right.sse[2]));
		result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(left, left, 0xff), right.sse[3]));
		return result;
	}
	
	Mat4 operator*(Mat4 a, Mat4 b)
	{
		Mat4 result;
		result.sse[0] = LinearCombineSSE(b.sse[0], a);
		result.sse[1] = LinearCombineSSE(b.sse[1], a);
		result.sse[2] = LinearCombineSSE(b.sse[2], a);
		result.sse[3] = LinearCombineSSE(b.sse[3], a);
		return result;
	}
	
	Vec4 operator*(Mat4 a, Vec4 b)
	{
		Vec4 result;
		result.sse = LinearCombineSSE(b.sse, a);
		return result;
	}
#else
	Mat4 operator*(Mat4 a, Mat4 b)
	{
		Mat4 result;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				result[i][j] = a[0][j] * b[i][0] + a[1][j] * b[i][1] + a[2][j] * b[i][2] + a[3][j] * b[i][3];
			}
		}
		return result;
	}
	
	Vec4 operator*(Mat4 a, Vec4 b)
	{
		Vec4 result;
		for (int i = 0; i < 4; ++i) result[i] = a[0][i] * b.x + a[1][i] * b.y + a[2][i] * b.z + a[3][i] * b.w;
		return result;
	}
#endif
	
	Vec4& Mat4::operator[](int i) {return cols[i];}
	const Vec4& Mat4::operator[](int i) const {return cols[i];}
	
	const Mat4 Mat4::Zero = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
	const Mat4 Mat4::Identity = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
	
	// Quat Implementation
	// ============================================================================
	
#ifdef GMATH_USE_SSE
	Quat::Quat(double fill) : sse(_mm_setr_ps(fill, fill, fill, fill)) {}
	Quat::Quat(double x, double y, double z, double w) : sse(_mm_setr_ps(x, y, z, w)) {}
	Quat::Quat(Vec4 values) : sse(values.sse) {}
	Quat::Quat(double data[4]) : sse(_mm_setr_ps(data[0], data[1], data[2], data[3])) {}
	
	Quat operator*(Quat a, Quat b)
    {
        Quat result;
        __m128 result_one = _mm_xor_ps(_mm_shuffle_ps(a.sse, a.sse, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.0, -0.0, 0.0, -0.0));
        __m128 result_two = _mm_shuffle_ps(b.sse, b.sse, _MM_SHUFFLE(0, 1, 2, 3));
        __m128 result_three = _mm_mul_ps(result_two, result_one);
        
        result_one = _mm_xor_ps(_mm_shuffle_ps(a.sse, a.sse, _MM_SHUFFLE(1, 1, 1, 1)) , _mm_setr_ps(0.0, 0.0, -0.0, -0.0));
        result_two = _mm_shuffle_ps(b.sse, b.sse, _MM_SHUFFLE(1, 0, 3, 2));
        result_three = _mm_add_ps(result_three, _mm_mul_ps(result_two, result_one));
        
        result_one = _mm_xor_ps(_mm_shuffle_ps(a.sse, a.sse, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.0, 0.0, 0.0, -0.0));
        result_two = _mm_shuffle_ps(b.sse, b.sse, _MM_SHUFFLE(2, 3, 0, 1));
        result_three = _mm_add_ps(result_three, _mm_mul_ps(result_two, result_one));
        
        result_one = _mm_shuffle_ps(a.sse, a.sse, _MM_SHUFFLE(3, 3, 3, 3));
        result_two = _mm_shuffle_ps(b.sse, b.sse, _MM_SHUFFLE(3, 2, 1, 0));
        result.sse = _mm_add_ps(result_three, _mm_mul_ps(result_two, result_one));
        return result;
    }
    
#else
	Quat::Quat(double fill) : x(fill), y(fill), z(fill), w(fill) {}
	Quat::Quat(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
	Quat::Quat(Vec4 values) : x(values.x), y(values.y), z(values.z), w(values.w) {}
	Quat::Quat(double data[4]) : data{data[0], data[1], data[2], data[3]} {}
	
	Quat operator*(Quat a, Quat b)
    {
        Quat result;
        result.x = (a.x * b.w) + (a.y * b.z) - (a.z * b.y) + (a.w * b.x);
        result.y = (-a.x * b.z) + (a.y * b.w) + (a.z * b.x) + (a.w * b.y);
        result.z = (a.x * b.y) - (a.y * b.x) + (a.z * b.w) + (a.w * b.z);
        result.w = (-a.x * b.x) - (a.y * b.y) - (a.z * b.z) + (a.w * b.w);
        return result;
    }
    
#endif
	
	Quat::Quat(Vec3 axis, double angle)
	{
		axis = Normalize(axis);
		double sin = Sin(angle / 2.0);
		xyz = axis * Vec3(sin);
		w = Cos(angle / 2.0);
	}
	
    // This method taken from Mike Day at Insomniac Games.
    // https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
    //
    // Note that as mentioned at the top of the paper, the paper assumes the matrix
    // would be *post*-multiplied to a vector to rotate it, meaning the matrix is
    // the transpose of what we're dealing with. But, because our matrices are
    // stored in column-major order, the indices *appear* to match the paper.
	Quat::Quat(Mat4 mat)
    {
        double t;
		Vec4 v;
        if (mat[2][2] < 0.0)
        {
            if (mat[0][0] > mat[1][1])
            {
                t = 1.0 + mat[0][0] - mat[1][1] - mat[2][2];
				v= {t, mat[0][1] + mat[1][0], mat[2][0] + mat[0][2], mat[1][2] - mat[2][1]};
            }
            else
            {
                t = 1.0 - mat[0][0] + mat[1][1] - mat[2][2];
				v= {mat[0][1] + mat[1][0], t, mat[1][2] + mat[2][1], mat[2][0] - mat[0][2]};
            }
        }
        else
        {
            if (mat[0][0] < -mat[1][1])
            {
                t = 1.0 - mat[0][0] - mat[1][1] + mat[2][2];
				v= {mat[2][0] + mat[0][2], mat[1][2] + mat[2][1], t, mat[0][1] - mat[1][0]};
            }
            else
            {
                t = 1.0 + mat[0][0] + mat[1][1] + mat[2][2];
				v = {mat[1][2] - mat[2][1], mat[2][0] - mat[0][2], mat[0][1] - mat[1][0], t};
            }
        }
        
		
        *this = Quat(v * Vec4(0.5f / Sqrt(t)));
    }

    Quat::Quat(Vec3 a, Vec3 b, Vec3 c)
    {
        Quat result = {};
        double t = a.x + b.y + c.z;
        if (t > 0.0)
        {
            double s = Sqrt(t + 1.0) * 2.0;
            result.x = (b.z - c.y) / s;
            result.y = (c.x - a.z) / s;
            result.z = (a.y - b.x) / s;
            result.w = 0.25f * s;
        }
        else if (a.x > b.y && a.x > c.z)
        {
            double s = Sqrt(1.0 + a.x - b.y - c.z) * 2.0;
            result.x = 0.25f * s;
            result.y = (a.y + b.x) / s;
            result.z = (c.x + a.z) / s;
            result.w = (b.z - c.y) / s;
        }
        else if (b.y > c.z)
        {
            double s = Sqrt(1.0 + b.y - a.x - c.z) * 2.0;
            result.x = (a.y + b.x) / s;
            result.y = 0.25f * s;
            result.z = (b.z + c.y) / s;
            result.w = (c.x - a.z) / s;
        }
        else
        {
            double s = Sqrt(1.0 + c.z - a.x - b.y) * 2.0;
            result.x = (c.x + a.z) / s;
            result.y = (b.z + c.y) / s;
            result.z = 0.25f * s;
            result.w = (a.y - b.x) / s;
        }

        *this = Normalize(result);
    }
    
	Quat& Quat::operator*=(Quat b)
	{
		*this = *this * b;
		return *this;
	}
	
	double& Quat::operator[](int i) {return data[i];}
	const double& Quat::operator[](int i) const {return data[i];}
	
	const Quat Quat::Zero = {0.0, 0.0, 0.0, 0.0};
    const Quat Quat::Identity = {0.0, 0.0, 0.0, 1.0};
    
#ifdef GMATH_USE_NAMESPACE
};
#endif
#endif // GMATH_IMPLEMENTATION

// Vec3 rotate_by_quat(Vec3 p, Quat q)
// {
// 	return 2.0 * Dot(q.xyz, p) * q.xyz + (q.w * q.w - Dot(q.xyz, q.xyz)) * p + 2.0 * q.w * Cross(q.xyz, p);
// }