#pragma once

#include "DirectXMath.h"
#include "cereal.h"
#include <stdexcept>
#include "string.h"

/***********************************************************************************************************************
													settings
************************************************************************************************************************/

#define LH
#define ARITHMETIC template<typename A, typename = std::enable_if_t<std::is_arithmetic<A>::value>>

/***********************************************************************************************************************
													definitions
************************************************************************************************************************/

#define PI			DirectX::XM_PI
#define PI2			DirectX::XM_2PI
#define EPS			1e-6f
#define ZERO_APPROX	0.0001f
#define VECTOR_UP	DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)

typedef DirectX::XMVECTOR	xmvector;
typedef unsigned long		hex_rgba;

/***********************************************************************************************************************
													functions
************************************************************************************************************************/

#define RADIANS(a)		DirectX::XMConvertToRadians(a)
#define DEGREES(a)		DirectX::XMConvertToDegrees(a)

#define TRANS_M(f3)		DirectX::XMMatrixTranslation(f3.x, f3.y, f3.z)
#define ROTATE_M(f3)	DirectX::XMMatrixRotationRollPitchYaw(f3.x, f3.y, f3.z)
#define SCALE_M(f3)		DirectX::XMMatrixScaling(f3.x, f3.y, f3.z)

ARITHMETIC inline	A		maxim		(A a, A b)						{ return (a > b) ? a : b; }
ARITHMETIC inline	A		minim		(A a, A b)						{ return (a < b) ? a : b; }

inline				float	clamp		(float min, float x, float max) { return (x < min) ? min			: ((x < max) ? x : max); }
ARITHMETIC inline	A		clamp		(A min, A x, A max)				{ return (x < min) ? min			: ((x < max) ? x : max); }
ARITHMETIC inline	A		wrap		(A min, A x, A max)				{ return (x < min) ? x + max - min	: ((x < max) ? x : x - max + min); }
ARITHMETIC inline	A		lerp		(const A& a, const A& b, A t)	{ return (1 - t) * a + t * b; }
inline				bool	is_zero		(float x)						{ return fabs(x) < ZERO_APPROX; }
inline				bool	non_zero	(float x)						{ return fabs(x) > ZERO_APPROX; }

/***********************************************************************************************************************
													float2
************************************************************************************************************************/

class float2 : public DirectX::XMFLOAT2 {
public:
	float2(DirectX::XMFLOAT2 c) : DirectX::XMFLOAT2(c) {}
	float2(float x, float y)	: DirectX::XMFLOAT2(x, y) {}
	float2(float s = 0)			: DirectX::XMFLOAT2(s, s) {}

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

#ifdef LH
	// LH Cross Product
	inline float operator%(float2 that) const { return y * that.x - x * that.y; }
#else
	// RH Cross Product
	inline float operator%(float2 that) const { return x * that.y - y * that.x; }
#endif

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
	inline float2 norm	()				{ const float m = mag(); if (non_zero(m)) return operator/=(m); else return 0; }
	inline float2 norm	() const		{ const float m = mag(); if (non_zero(m)) return operator/(m);  else return 0; }
	inline float2 abs	() const		{ return { fabsf(x), fabsf(y) }; }
	inline float2 rotate(float angle_degrees);

	inline explicit operator bool				() const { return  x || y ; }
	inline			operator DirectX::XMFLOAT2	() const { return {x,   y}; }
	inline			operator string				() const { return string("{ ", x, ", ", y, " }"); }
	
	SERIALIZE(x, y)
};

inline float	dot				(float2 a,			float2 b						)	{ return { a.x * b.x + a.y * b.y }; }
inline float	tangent_cross	(float2 a,			float2 b						)	{ return a.x * b.y - a.y * b.x; }
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

/***********************************************************************************************************************
													float3
************************************************************************************************************************/

class float3 : public DirectX::XMFLOAT3 {
public:
	float3(DirectX::XMFLOAT3 c)			: DirectX::XMFLOAT3(c) {}
	float3(float x, float y, float z)	: DirectX::XMFLOAT3(x, y, z) {}
	float3(float2 f, float z = 0)		: DirectX::XMFLOAT3(f.x, f.y, z) {}
	float3(float s = 0)					: DirectX::XMFLOAT3(s, s, s) {}
	float3(DirectX::XMVECTOR v)			{ DirectX::XMStoreFloat3(this, v); }

	inline float2 xy() { return { x, y }; }
	inline float2 xz() { return { x, z }; }
	inline float2 yz() { return { y, z }; }

	inline				float3 operator+(const float3& that			) const { return { x + that.x,	y + that.y,		z + that.z		}; }
	inline				float3 operator*(const float3& that			) const { return { x * that.x,	y * that.y,		z * that.z		}; }
	inline				float3 operator*(const DirectX::XMMATRIX& m	) const { return DirectX::XMVector4Transform((xmvector)*this, m	); }
	ARITHMETIC inline	float3 operator*(const A& scale				) const { return { x * scale,	y * scale,		z * scale		}; }
	inline				float3 operator/(const float3& that			) const { return { x / that.x,	y / that.y,		z / that.z		}; }
	ARITHMETIC inline	float3 operator/(const A& scale				) const { return { x / scale,	y / scale,		z / scale		}; }
	inline				float3 operator-(							) const { return { -x,		   -y,			   -z				}; }
	inline				float3 operator-(const float3& that			) const { return operator+(-that); }

	inline				friend float3 operator*(const float& lh,	const float3& rh) { return rh * lh; }
	ARITHMETIC inline	friend float3 operator*(const A& lh,		const float3& rh) { return rh * lh; }

	inline				float3 operator+=(float3 that	) { return *this = operator+(that);  }
	inline				float3 operator-=(float3 that	) { return *this = operator-(that);  }
	inline				float3 operator*=(float3 that	) { return *this = operator*(that);  }
	ARITHMETIC inline	float3 operator*=(A scale		) { return *this = operator*(scale); }
	inline				float3 operator/=(float3 that	) { return *this = operator/(that);  }
	ARITHMETIC inline	float3 operator/=(A scale		) { return *this = operator/(scale); }


#ifdef LH
	// LH Cross Product
	inline float3 operator%(float3 that) const { return { y * that.z - z * that.y, z * that.x - x * that.z, x * that.y - y * that.x }; }
	inline float3 to_euler() const { return { atan2(x, z), -asin(y), 0 }; }

#else
	// RH Cross Product
	inline float3 operator%(float3 that) const { return { y * that.z - z * that.y, x * that.z - z * that.x, x * that.y - y * that.x }; }
	inline float3 to_euler() const { return { atan2(-x, -z), asin(y), 0 }; }
#endif

	inline bool operator==(const float3& that) const { return x == that.x && y == that.y && z == that.z; }
	inline bool operator!=(const float3& that) const { return x != that.x || y != that.y || z != that.z; }
	inline bool operator< (const float3& that) const { return x <  that.x && y <  that.y && z <  that.z; }
	inline bool operator<=(const float3& that) const { return x <= that.x && y <= that.y && z <= that.z; }
	inline bool operator> (const float3& that) const { return x >  that.x || y >  that.y || z >  that.z; }
	inline bool operator>=(const float3& that) const { return x >= that.x || y >= that.y || z >= that.z; }
	
	inline		 float& operator[](unsigned short i)			{ switch (i) { case 0: return x; case 1: return y; case 2: return z; default: throw std::out_of_range("Invalid Index"); } }
	inline const float& operator[](unsigned short i) const		{ switch (i) { case 0: return x; case 1: return y; case 2: return z; default: throw std::out_of_range("Invalid Index"); } }

	inline float  mag_sq	() const	{ return (x * x + y * y + z * z); }
	inline float  mag		() const	{ return sqrtf(mag_sq()); }
	inline float3 norm		()			{ const float m = mag(); if (non_zero(m)) return operator/=(m); else return 0; }
	inline float3 norm		() const	{ const float m = mag(); if (non_zero(m)) return operator/(m);  else return 0; }
	inline float3 abs		() const	{ return { fabsf(x), fabsf(y), fabsf(z) }; }

	inline explicit operator bool				() const { return x || y || z; }
	inline			operator DirectX::XMFLOAT3	() const { return { x, y, z }; }
	inline			operator DirectX::XMFLOAT4	() const { return { x, y, z, 1 }; }
	inline			operator string				() const { return string("{ ", x, ", ", y, ", ", z, " }"); }

	explicit inline operator xmvector() const { return DirectX::XMVectorSet(x, y, z, 1.0f); }

	SERIALIZE(x, y, z)
};

inline float	dot		(float3 a,			float3 b						)	{ return { a.x * b.x + a.y * b.y + a.z * b.z }; }
inline float3   clamp	(float3 min,		float3 x,			float3 max	)	{ return { clamp(min.x, x.x, max.x),	clamp(min.y, x.y, max.y),	clamp(min.z, x.z, max.z) }; }
inline float3   wrap	(float3 min,		float3 x,			float3 max	)	{ return { wrap(min.x, x.x, max.x),		wrap(min.y, x.y, max.y),	wrap(min.z, x.z, max.z) }; }
inline float3	lerp	(const float3& a,	const float3& b,	float t		)	{ return a * (1.0f - t) + b * t; }

inline float	dist	(const float3& a, const float3& b)		{ return (a - b).mag(); }

inline void update_maximum(float3& max, float3 vertex) {
	if (vertex.x > max.x) max.x = vertex.x;
	if (vertex.y > max.y) max.y = vertex.y;
	if (vertex.z > max.z) max.z = vertex.z;
}

inline void update_minimum(float3& min, float3 vertex) {
	if (vertex.x < min.x) min.x = vertex.x;
	if (vertex.y < min.y) min.y = vertex.y;
	if (vertex.z < min.z) min.z = vertex.z;
}

inline float3 maximize(const float3& A, const float3& B) {
	float3 C;
	for (int i = 0; i < 3; i++) { C[i] = maxim(A[i], B[i]); }
	return C;
}

inline float3 minimize(const float3& A, const float3& B) {
	float3 C;
	for (int i = 0; i < 3; i++) { C[i] = minim(A[i], B[i]); }
	return C;
}

inline float3 to_euler(const float3& vector) {
	return {
		atan2f(vector.x, vector.z),
		atan2f(-vector.y, sqrtf(vector.x * vector.x + vector.z * vector.z)),
		0
	};
}

inline DirectX::XMFLOAT3 break_quaternion(const DirectX::XMVECTOR& q) {
	DirectX::XMMATRIX M{ DirectX::XMMatrixRotationQuaternion(q) };

	return {
		asinf(-M.r[2].m128_f32[1]),						// pitch
		atan2f(M.r[2].m128_f32[0], M.r[2].m128_f32[2]), // yaw
		atan2f(M.r[0].m128_f32[1], M.r[1].m128_f32[1])	// roll
	};
}

inline DirectX::XMFLOAT3 break_quaternion(const DirectX::XMFLOAT4& q) { return break_quaternion(DirectX::XMLoadFloat4(&q)); }

/***********************************************************************************************************************
													float3x3
************************************************************************************************************************/

class float3x3 {
public:
	float3 r[3];

	float3x3() = default;

	float3x3(const float3& a, const float3& b, const float3& c) {
		r[0] = a;
		r[1] = b;
		r[2] = c;
	}

	float3x3(const float3& euler_angles) {
		float3 c = { cosf(euler_angles.x), cosf(euler_angles.y), cosf(euler_angles.z) };
		float3 s = { sinf(euler_angles.x), sinf(euler_angles.y), sinf(euler_angles.z) };

		r[0] = { c.y * c.z,						c.y * s.z,					-s.y };
		r[1] = { s.x * s.y * c.z - c.x * s.z,	s.x * s.y * s.z + c.x * c.z, s.x * c.y };
		r[2] = { c.x * s.y * c.z + s.x * s.z,	c.x * s.y * s.z - s.x * c.z, c.x * c.y };
	}

	float3x3(bool) {
		r[0] = { 1, 0, 0 };
		r[1] = { 0, 1, 0 };
		r[2] = { 0, 0, 1 };
	}

	float3& operator[](int i) { return r[i]; }
	const float3& operator[](int i) const { return r[i]; }

	float3 x		() const { return { r[0].x, r[1].x, r[2].x }; }
	float3 y		() const { return { r[0].y, r[1].y, r[2].y }; }
	float3 z		() const { return { r[0].z, r[1].z, r[2].z }; }

	float3 right	() const { return r[0]; }
	float3 up		() const { return r[1]; }
	float3 forward	() const { return r[2]; }

	float3x3 transpose() const {
		float3x3 t;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				t.r[i][j] = r[j][i];
			}
		}
		return t;
	}

	float3 rotate(const float3& t) const { return { dot(t, r[0]), dot(t, r[1]), dot(t, r[2]) }; }
	float3 inv_rotate(const float3& t) const { return transpose().rotate(t); }

	float3 to_euler() const { return { asin(clamp(-1.0f, r[1].z, 1.0f)), atan2(-r[2].x, -r[2].z), atan2(-r[0].y, r[1].y) }; }
};

inline float3x3 face_to(const float3& F) {
	float3 R, U, H = { 0, 1, 0 }; // U Hint
	if (fabsf(dot(H, F)) > EPS) {
		R = (H % F).norm();
		U = (F % R);
	}
	else {
		H = { 1, 0, 0 }; // R Hint
		U = (H % F).norm();
		R = (F % U);
	}
	return { R, U, -F };
}

inline float3x3 head_to(const float3& U) {
	float3 R, F, H = { 0, 0, -1 }; // F Hint
	if (fabsf(dot(H, U)) > EPS) {
		R = (H % U).norm();
		F = (U % R);
	}
	else {
		H = { 1, 0, 0 }; // R Hint
		F = (H % U).norm();
		R = (U % F);
	}
	return { R, U, -F };
}

inline float3x3 side_to(const float3& R) {
	float3 F, U, H = { 0, 1, 0 }; // U Hint
	if (fabsf(dot(H, R)) > EPS) {
		F = (H % R).norm();
		U = (R % F);
	}
	else {
		H = { 0, 0, -1 }; // F Hint
		U = (H % R).norm();
		F = (R % U);
	}
	return { R, U, -F };
}

/***********************************************************************************************************************
													color
************************************************************************************************************************/

class color {
public:
	float r, g, b, a;

	color()						: r(1),		g(1),	b(1),	a(1)	{}
	color(DirectX::XMFLOAT4 c)	: r(c.x),	g(c.y), b(c.z), a(c.w)	{}
	color(float r, float g, float b, float a = 1.0f) : r(clamp(0, r, 1)), g(clamp(0, g, 1)), b(clamp(0, b, 1)), a(clamp(0, a, 1)) {}
	color(hex_rgba hex) : 
		r(((hex >> 24) & 0xff) / 255.0f), 
		g(((hex >> 16) & 0xff) / 255.0f), 
		b(((hex >>  8) & 0xff) / 255.0f), 
		a(((hex		 ) & 0xff) / 255.0f) {}

	inline color operator*(color that) const { return { r * that.r,	g * that.g,	b * that.b, a * that.a }; }

	inline operator DirectX::XMFLOAT4() const { return { r, g, b, a }; }
	inline operator hex_rgba() const { return 
		((int(r * 255) & 0xff) << 24) |
		((int(g * 255) & 0xff) << 16) |
		((int(b * 255) & 0xff) << 8 ) |
		((int(a * 255) & 0xff)); 
	}

	SERIALIZE(r, g, b, a)
};

#define WHITE	{1, 1, 1}
#define RED		{1, 0, 0}
#define GREEN	{0, 1, 0}
#define BLUE	{0, 0, 1}
#define YELLOW	{1, 1, 0}
#define AQUA    {0, 1, 1}
#define PINK    {1, 0, 1}
#define BLACK	{0, 0, 0}

/***********************************************************************************************************************
													float4
************************************************************************************************************************/


class float4 : public DirectX::XMFLOAT4 {
public:
	float4(DirectX::XMFLOAT4 c)					: DirectX::XMFLOAT4(c) {}
	float4(float x, float y, float z, float w)	: DirectX::XMFLOAT4(x, y, z, w) {}
	float4(float3 f, float w = 0)				: DirectX::XMFLOAT4(f.x, f.y, f.z, w) {}
	float4(float2 f, float2 ff = 0)				: DirectX::XMFLOAT4(f.x, f.y, ff.x, ff.y) {}
	float4(float2 f, float z, float w)			: DirectX::XMFLOAT4(f.x, f.y, z, w) {}
	float4(float s = 0)							: DirectX::XMFLOAT4(s, s, s, s) {}
	float4(color c)								: DirectX::XMFLOAT4(c.r, c.g, c.b, c.a) {}

	inline float2 xy() { return { x, y }; }
	inline float2 xz() { return { x, z }; }
	inline float2 xw() { return { x, w }; }
	inline float2 yz() { return { y, z }; }
	inline float2 yw() { return { y, w }; }
	inline float2 zw() { return { z, w }; }

	inline float3 xyz() { return { x, y, z }; }
	inline float3 xyw() { return { x, y, w }; }
	inline float3 xzw() { return { x, z, w }; }
	inline float3 yzw() { return { y, z, w }; }

	inline				float4 operator+(float4 that	) const { return { x + that.x,	y + that.y,		z + that.z,  w + that.w		}; }
	inline				float4 operator*(float4 that	) const { return { x * that.x,	y * that.y,		z * that.z,  w * that.w		}; }
	ARITHMETIC inline	float4 operator*(A scale		) const { return { x * scale,	y * scale,		z * scale,	 w * scale		}; }
	inline				float4 operator/(float4 that	) const { return { x / that.x,	y / that.y,		z / that.z,  w / that.w		}; }
	ARITHMETIC inline	float4 operator/(A scale		) const { return { x / scale,	y / scale,		z / scale,   w / scale		}; }
	inline				float4 operator-(				) const { return { -x,		   -y,			   -z,			-w				}; }
	inline				float4 operator-(float4 that	) const { return operator+(-that); }

	inline				friend float4 operator*(const float& lh,	const float4& rh) { return rh * lh; }
	ARITHMETIC inline	friend float4 operator*(const A& lh,		const float4& rh) { return rh * lh; }

	inline				float4 operator+=(float4 that	) { return *this = operator+(that);  }
	inline				float4 operator-=(float4 that	) { return *this = operator-(that);  }
	inline				float4 operator*=(float4 that	) { return *this = operator*(that);  }
	ARITHMETIC inline	float4 operator*=(A scale		) { return *this = operator*(scale); }
	inline				float4 operator/=(float4 that	) { return *this = operator/(that);  }
	ARITHMETIC inline	float4 operator/=(A scale		) { return *this = operator/(scale); }

	inline bool operator==(const float4& that) const { return x == that.x && y == that.y && z == that.z && w == that.w; }
	inline bool operator!=(const float4& that) const { return x != that.x || y != that.y || z != that.z || w != that.w; }
	inline bool operator< (const float4& that) const { return x <  that.x && y <  that.y && z <  that.z && w <  that.w; }
	inline bool operator<=(const float4& that) const { return x <= that.x && y <= that.y && z <= that.z && w <= that.w; }
	inline bool operator> (const float4& that) const { return x >  that.x || y >  that.y || z >  that.z || w >  that.w; }
	inline bool operator>=(const float4& that) const { return x >= that.x || y >= that.y || z >= that.z || w >= that.w; }

	inline		 float& operator[](unsigned short i)		{ switch (i) { case 0: return x; case 1: return y; case 2: return z; case 3: return w; default: throw std::out_of_range("Invalid Index"); } }
	inline const float& operator[](unsigned short i) const	{ switch (i) { case 0: return x; case 1: return y; case 2: return z; case 3: return w; default: throw std::out_of_range("Invalid Index"); } }
	
	inline float  mag_sq() const	{ return (x * x + y * y + z * z + w * w); }
	inline float  mag	() const	{ return sqrtf(mag_sq()); }
	inline float4 norm	()			{ const float m = mag(); if (non_zero(m)) return operator/=(m); else return 0; }
	inline float4 norm	() const	{ const float m = mag(); if (non_zero(m)) return operator/(m);  else return 0; }
	inline float4 abs	() const	{ return { fabsf(x), fabsf(y), fabsf(z), fabsf(w) }; }

	inline explicit operator bool				() const { return x || y || z || w; }
	inline			operator color				() const { return { x, y, z, w }; }
	inline			operator DirectX::XMFLOAT4	() const { return { x, y, z, w }; }
	inline			operator string				() const { return string("{ ", x, ", ", y, ", ", z, ", ", w, " }"); }

	SERIALIZE(x, y, z, w)
};

inline float	dot		(float4 a,			float4 b						)	{ return { a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w }; }
inline float4   clamp	(float4 min,		float4 x,			float4 max	)	{ return { clamp(min.x, x.x, max.x),	clamp(min.y, x.y, max.y),	clamp(min.z, x.z, max.z),	clamp(min.w, x.w, max.w) }; }
inline float4   wrap	(float4 min,		float4 x,			float4 max	)	{ return { wrap(min.x, x.x, max.x),		wrap(min.y, x.y, max.y),	wrap(min.z, x.z, max.z),	wrap(min.w, x.w, max.w) }; }
inline float4	lerp	(const float4& a,	const float4& b,	float t		)	{ return (1 - t) * a + t * b; }

inline float	dist	(const float4& a, const float4& b) { return (a - b).mag(); }

/***********************************************************************************************************************
*													float4x4
************************************************************************************************************************/

typedef DirectX::XMFLOAT4X4 float4x4;

#define MATRIX_ID {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}
constexpr float4x4 matrix_id = MATRIX_ID;

#define MATRIX_EMPTY {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
constexpr float4x4 matrix_empty = MATRIX_EMPTY;

inline bool is_empty(float4x4& m) {
	return !(m._11 || m._12 || m._13 || m._14 || m._21 || m._22 || m._23 || m._24 || m._31 || m._32 || m._33 || m._34 || m._41 || m._42 || m._43 || m._44);
}

namespace cereal {
	template <class T>
	inline void serialize(T& archive, float4x4& m) {
		archive(
			NVP(m, _11), NVP(m, _12), NVP(m, _13), NVP(m, _14),
			NVP(m, _21), NVP(m, _22), NVP(m, _23), NVP(m, _24),
			NVP(m, _31), NVP(m, _32), NVP(m, _33), NVP(m, _34),
			NVP(m, _41), NVP(m, _42), NVP(m, _43), NVP(m, _44)
		);
	}
}

/***********************************************************************************************************************
													matrix
************************************************************************************************************************/

typedef DirectX::XMMATRIX matrix;

/***********************************************************************************************************************
													transform
************************************************************************************************************************/

class transform {
private:
	mutable matrix mat;
	mutable bool needs_update = true;

	//if neccesarry
	//float4x4 f44;

	float3 pos	;
	float3 scale;
	float3 angle; // convert to quaternion?
	float3 pivot;
	 
public:
	transform(const matrix& decomp) : mat(MATRIX_ID) {
		DirectX::XMVECTOR p, s, a;
		DirectX::XMMatrixDecompose(&s, &a, &p, decomp);
		DirectX::XMStoreFloat3(&pos, p);
		DirectX::XMStoreFloat3(&scale, s);
		angle = break_quaternion(a);
	}

	transform(const float4x4& decomp) : transform(DirectX::XMLoadFloat4x4(&decomp)) {}

#ifdef _DEBUG

	float3& get_pos_ref() { return pos; }
	float3& get_scl_ref() { return scale; }
	float3& get_ang_ref() { return angle; }
	float3& get_pvt_ref() { return pivot; }

#endif // _DEBUG

	float3 get_mid() const { return pos; }
	float3 get_pos() const { return pos + pivot; }
	float3 get_scl() const { return scale; }
	float3 get_ang() const { return angle; }
	float3 get_pvt() const { return pivot; }

	DirectX::XMVECTOR get_quat() const { return DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z); }

	void set_pos(float3 x) { pos   = x - pivot; needs_update = true; }
	void set_scl(float3 x) { scale = x;			needs_update = true; }
	void set_ang(float3 x) { angle = x;			needs_update = true; }
	void set_pvt(float3 x) { pivot = x;			needs_update = true; }

	void add_pos(float3 x) { pos   += x;		needs_update = true; }
	void add_scl(float3 x) { scale += x;		needs_update = true; }
	void add_ang(float3 x) { angle += x;		needs_update = true; }
	void add_pvt(float3 x) { pivot += x;		needs_update = true; }

	void mlt_pos(float3 x) { pos   *= x;		needs_update = true; }
	void mlt_scl(float3 x) { scale *= x;		needs_update = true; }
	void mlt_ang(float3 x) { angle *= x;		needs_update = true; }
	void mlt_pvt(float3 x) { pivot *= x;		needs_update = true; }

	void force_update() const { needs_update = true; }

	transform(float3 p = {0}, float3 s = {1}, float3 a = {0}) : pos(p), scale(s), angle(a), pivot(0), mat() {}
	transform(float x, float y, float z) : transform(float3(x, y, z)) {}

	inline void update_matrix() const { if (!needs_update) { return; } mat = TRANS_M(-pivot) * SCALE_M(scale) * ROTATE_M(angle) * TRANS_M((pos + pivot)); needs_update = false; }
	//inline void update_float() { update_matrix(); DirectX::XMStoreFloat4x4(&f44, mat); }

	inline operator matrix		() const { update_matrix(); return mat; }
	inline operator matrix&		() const { update_matrix(); return mat; }
	inline operator matrix*		() const { update_matrix(); return &mat; }

	inline transform operator+  (const transform& o) { return { pos + o.pos, scale * o.scale, angle + o.angle }; }
	inline transform operator+= (const transform& o) { *this = operator+(o); needs_update = true; return *this; }

	inline matrix operator* (transform& o)		{ return (matrix)*this * (matrix)o; }
	inline matrix operator* (const matrix& m)	{ return (matrix)*this * m; }

	inline operator float4x4		()				const { update_matrix(); float4x4 out;	DirectX::XMStoreFloat4x4(&out, mat); return out;	}
	inline void		get_float4x4	(float4x4& out) const { update_matrix();				DirectX::XMStoreFloat4x4(&out, mat);				}
	//inline operator float4x4& () { update_float(); return f44; } 

	SERIALIZE(pos, scale, angle, pivot)
};

inline transform lerp(const transform& a, const transform& b, float t) { 
	assert(a.get_pvt() == b.get_pvt());
	transform o; 
	o.set_pvt(a.get_pvt());
	o.set_pos(lerp(a.get_pos(), b.get_pos(), t)); 
	o.set_scl(lerp(a.get_scl(), b.get_scl(), t));
	o.set_ang(break_quaternion(DirectX::XMQuaternionSlerp(a.get_quat(), b.get_quat(), t)));
	return o; 
}

inline matrix lerp(const matrix& a, const matrix& b, float t) {
	return (matrix)lerp(transform(a), transform(b), t);
}

inline float4x4 lerp(const float4x4& a, const float4x4& b, float t) {
	return (float4x4)lerp(transform(a), transform(b), t);
}

inline transform position_transform	(float3 position) { return transform(position,	1,		0		); }
inline transform scale_transform	(float3 scale	) { return transform(0,			scale,	0		); }
inline transform rotation_transform	(float3 rotation) { return transform(0,			1,		rotation); }

/***********************************************************************************************************************
													triangle
************************************************************************************************************************/

class triangle {
public:
	float3 A;
	float3 B;
	float3 C;

	inline		 float3& operator[](unsigned short i)			{ switch (i) { case 0: return A; case 1: return B; case 2: return C; default: throw std::out_of_range("Invalid Index"); } }
	inline const float3& operator[](unsigned short i) const		{ switch (i) { case 0: return A; case 1: return B; case 2: return C; default: throw std::out_of_range("Invalid Index"); } }

	inline triangle operator*(transform t) const { triangle out; for (int i = 0; i < 3; i++) { out[i] = this->operator[](i) * t; } return out; }

	triangle() {}
	triangle(const float3& a, const float3& b, const float3& c) : A(a), B(b), C(c) {}

	// Assumes CW Winding
	float3 norm() const { return ((B - A) % (C - A)).norm(); }
	float3 centroid() const { return (A + B + C) / 3.0f; }

	SERIALIZE(A, B, C)
};

inline bool triangle_intersection(const triangle& tri, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal) {
	const float3 AB = tri.B - tri.A;
	const float3 AC = tri.C - tri.A;
	
	const float3& normal = (AB % AC).norm();
	if (out_int_normal) *out_int_normal = normal;

	const float td = dot(normal, ray);
	if (!non_zero(td)) { return false; } // ray is parallel to plane

	const float t = (dot(normal, tri.A) - dot(normal, origin)) / td;
	if (t < EPS) { return false; } // plane is behind the origin.

	const float3& P = origin + (ray * t);
	if (out_int_point) *out_int_point = P;

	const float3 AP = P - tri.A;

	const float ab2	 = dot(AB, AB);
	const float abac = dot(AB, AC);
	const float abap = dot(AB, AP);
	const float ac2  = dot(AC, AC);
	const float acap = dot(AC, AP);

	const float d = ab2 * ac2 - abac * abac;
	if (!non_zero(d)) { return false; } // degenerate triangle

	const float u = (ac2 * abap - abac * acap) / d;
	const float v = (ab2 * acap - abac * abap) / d;

	return (u >= 0 && v >= 0 && u + v <= 1);
}
