#pragma once

#include "cereal.h"
#include <stdexcept>
#include "string.h"

#include "math_main.h"

/***********************************************************************************************************************
													float2
************************************************************************************************************************/

class float2 : public DirectX::XMFLOAT2 {
public:
	float2(DirectX::XMFLOAT2 c)		: DirectX::XMFLOAT2(c)		{}
	float2(float x, float y)		: DirectX::XMFLOAT2(x, y)	{}
	explicit float2(float s = 0)	: DirectX::XMFLOAT2(s, s)	{}

	inline float2 oy() const { return { 0, y }; }
	inline float2 xo() const { return { x, 0 }; }

	inline				float2 operator+(const float2& that	) const { return { x + that.x,	y + that.y		};	}
	inline				float2 operator*(const float2& that ) const { return { x * that.x,	y * that.y		};	}
	ARITHMETIC inline	float2 operator*(const A& scale		) const { return { x * scale,	y * scale		};	}
	inline				float2 operator/(const float2& that ) const { return { x / that.x,	y / that.y		};	}
	ARITHMETIC inline	float2 operator/(const A& scale		) const { return { x / scale,	y / scale		};	}
	inline				float2 operator-(					) const { return {-x,		   -y				};	}
	inline				float2 operator-(const float2& that	) const { return operator+(that.operator-());		}

	inline				friend float2 operator*(const float& lh,	const float2& rh) { return rh * lh; }
	ARITHMETIC inline	friend float2 operator*(const A& lh,		const float2& rh) { return rh * lh; }

	inline				float2 operator+=(const float2& that) { return *this = operator+(that);  }
	inline				float2 operator-=(const float2& that) { return *this = operator-(that);  }
	inline				float2 operator*=(const float2& that) { return *this = operator*(that);  }
	ARITHMETIC inline	float2 operator*=(const A& scale	) { return *this = operator*(scale); }
	inline				float2 operator/=(const float2& that) { return *this = operator/(that);	 }
	ARITHMETIC inline	float2 operator/=(const A& scale	) { return *this = operator/(scale); }

//#ifdef LH
//	// LH Cross Product
//	inline float operator%(float2 that) const { return y * that.x - x * that.y; }
//#else
//	// RH Cross Product
	inline float operator%(float2 that) const { return x * that.y - y * that.x; }
//#endif

	inline bool operator==(const float2& that) const { return x == that.x && y == that.y; }
	inline bool operator!=(const float2& that) const { return x != that.x || y != that.y; }
	inline bool operator< (const float2& that) const { return x <  that.x && y <  that.y; }
	inline bool operator<=(const float2& that) const { return x <= that.x && y <= that.y; }
	inline bool operator> (const float2& that) const { return x >  that.x || y >  that.y; }
	inline bool operator>=(const float2& that) const { return x >= that.x || y >= that.y; }

	inline		 float& operator[](unsigned short i)			{ switch (i) { case 0: return x; case 1: return y; default: throw std::out_of_range("Invalid Index"); } }
	inline const float& operator[](unsigned short i) const		{ switch (i) { case 0: return x; case 1: return y; default: throw std::out_of_range("Invalid Index"); } }

	inline float  mag_sq() const		{ return (x * x + y * y); }
	inline float  mag	() const		{ return sqrtf(mag_sq()); }
	//inline float2 norm()				{ const float m = mag(); if (non_zero(m)) return operator/=(m); else return 0; }
	inline float2 norm	() const		{ const float m = mag(); if (non_zero(m)) return operator/(m);  else return float2{0}; }
	inline float2 floor () const		{ return { floorf(x), floorf(y) }; }
	inline float2 ceil	() const		{ return { ceilf(x), ceilf(y) }; }
	inline float2 abs	() const		{ return { fabsf(x), fabsf(y) }; }
	inline float2 rotate(float angle_degrees);

	inline explicit operator bool				() const { return  x || y ; }
	inline			operator DirectX::XMFLOAT2	() const { return {x,   y}; }
	inline			operator string				() const { return string("{ ", x, ", ", y, " }"); }
	
	SERIALIZE(x, y)
};

inline float	dot				(float2 a,			float2 b						)	{ return { a.x * b.x + a.y * b.y }; }
//inline float	tangent_cross	(float2 a,			float2 b						)	{ return a.x * b.y - a.y * b.x; }
inline float2   clamp			(float2 min,		float2 x,			float2 max	)	{ return { clamp(min.x, x.x, max.x),	clamp(min.y, x.y, max.y)	}; }
inline float2   wrap			(float2 min,		float2 x,			float2 max	)	{ return { wrap(min.x, x.x, max.x),		wrap(min.x, x.x, max.x)		}; }
inline float2	lerp			(const float2& a,	const float2& b,	float t		)	{ return a * (1 - t) + b * t; }

inline float	dist	(const float2& a, const float2& b) { return (a - b).mag(); }

inline float2 float2::rotate(float angle) { return { dot(*this,{ cosf(angle), -sinf(angle) }), dot(*this,{ sinf(angle), cosf(angle) }) }; }

// Center Points
// T - top		| L - left
// C - center   | C - center
// B - bottom   | R - right

#define C_TL float2{ -1,   1  }
#define C_TC float2{  0,   1  }
#define C_TR float2{  1,   1  }
#define C_CL float2{ -1,   0  }
#define C_CC float2{  0,   0  }
#define C_CR float2{  1,   0  }
#define C_BL float2{ -1,  -1  }
#define C_BC float2{  0,  -1  }
#define C_BR float2{  1,  -1  }