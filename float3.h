#pragma once

#include "float2.h"

/***********************************************************************************************************************
													float3
************************************************************************************************************************/

class float3 : public DirectX::XMFLOAT3 {
public:
	float3(DirectX::XMFLOAT3 c)			: DirectX::XMFLOAT3(c) {}
	float3(float x, float y, float z)	: DirectX::XMFLOAT3(x, y, z) {}
	float3(float2 f, float z = 0)		: DirectX::XMFLOAT3(f.x, f.y, z) {}
	explicit float3(float s = 0)		: DirectX::XMFLOAT3(s, s, s) {}
	float3(DirectX::XMVECTOR v)			{ DirectX::XMStoreFloat3(this, v); }

	inline float2 xy() const { return { x, y }; }
	inline float2 xz() const { return { x, z }; }
	inline float2 yz() const { return { y, z }; }

	inline float3 xyo() const { return { x, y, 0 }; }
	inline float3 xoz() const { return { x, 0, z }; }
	inline float3 oyz() const { return { 0, y, z }; }
	inline float3 xoo() const { return { x, 0, 0 }; }
	inline float3 oyo() const { return { 0, y, 0 }; }
	inline float3 ooz() const { return { 0, 0, z }; }

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

	// cross
	inline float3 operator%(float3 that) const { return { y * that.z - z * that.y, z * that.x - x * that.z, x * that.y - y * that.x }; } 

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
	//inline float3 norm	()			{ const float m = mag(); if (non_zero(m)) return operator/=(m); else return 0; }
	inline float3 norm		() const	{ const float m = mag(); if (non_zero(m)) return operator/(m);  else return float3{0}; }
	inline float3 floor		() const	{ return { floorf(x), floorf(y), floorf(z) }; }
	inline float3 ceil		() const	{ return { ceilf(x), ceilf(y), ceilf(z) }; }
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

//inline float3 to_euler(const float3& vector) {
//	return {
//		atan2f(vector.x, vector.z),
//		atan2f(-vector.y, sqrtf(vector.x * vector.x + vector.z * vector.z)),
//		0
//	};
//}