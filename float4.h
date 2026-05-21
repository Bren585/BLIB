#pragma once

#include "color.h"

/***********************************************************************************************************************
													float4
************************************************************************************************************************/

class float4 : public DirectX::XMFLOAT4 {
public:
	float4(DirectX::XMFLOAT4 c)					: DirectX::XMFLOAT4(c) {}
	float4(float x, float y, float z, float w)	: DirectX::XMFLOAT4(x, y, z, w) {}
	float4(float3 f, float w = 0)				: DirectX::XMFLOAT4(f.x, f.y, f.z, w) {}
	float4(float2 f, float2 ff = float2{0})		: DirectX::XMFLOAT4(f.x, f.y, ff.x, ff.y) {}
	float4(float2 f, float z, float w)			: DirectX::XMFLOAT4(f.x, f.y, z, w) {}
	explicit float4(float s = 0)				: DirectX::XMFLOAT4(s, s, s, s) {}
	float4(color c)								: DirectX::XMFLOAT4(c.r, c.g, c.b, c.a) {}
	float4(DirectX::XMVECTOR v)					{ DirectX::XMStoreFloat4(this, v); }

	inline float2 xy() const { return { x, y }; }
	inline float2 xz() const { return { x, z }; }
	inline float2 xw() const { return { x, w }; }
	inline float2 yz() const { return { y, z }; }
	inline float2 yw() const { return { y, w }; }
	inline float2 zw() const { return { z, w }; }

	inline float3 xyz() const { return { x, y, z }; }
	inline float3 xyw() const { return { x, y, w }; }
	inline float3 xzw() const { return { x, z, w }; }
	inline float3 yzw() const { return { y, z, w }; }

	inline float4 xyzo() const { return { x, y, z, 0 }; }
	inline float4 xyow() const { return { x, y, 0, w }; }
	inline float4 xozw() const { return { x, 0, z, w }; }
	inline float4 oyzw() const { return { 0, y, z, w }; }
	inline float4 xoow() const { return { x, 0, 0, w }; }
	inline float4 oyow() const { return { 0, y, 0, w }; }
	inline float4 oozw() const { return { 0, 0, z, w }; }
	inline float4 xyoo() const { return { x, y, 0, 0 }; }
	inline float4 xozo() const { return { x, 0, z, 0 }; }
	inline float4 oyzo() const { return { 0, y, z, 0 }; }
	inline float4 xooo() const { return { x, 0, 0, 0 }; }
	inline float4 oyoo() const { return { 0, y, 0, 0 }; }
	inline float4 oozo() const { return { 0, 0, z, 0 }; }
	inline float4 ooow() const { return { 0, 0, 0, w }; }

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
	//inline float4 norm()			{ const float m = mag(); if (non_zero(m)) return operator/=(m); else return 0; }
	inline float4 norm	() const	{ const float m = mag(); if (non_zero(m)) return operator/(m);  else return float4(0); }
	inline float4 floor	() const	{ return { floorf(x), floorf(y), floorf(z), floorf(w) }; }
	inline float4 ceil	() const	{ return { ceilf(x), ceilf(y), ceilf(z), ceilf(w) }; }
	inline float4 abs	() const	{ return { fabsf(x), fabsf(y), fabsf(z), fabsf(w) }; }

	inline explicit operator bool				() const { return x || y || z || w; }
	inline			operator color				() const { return { x, y, z, w }; }
	inline			operator DirectX::XMFLOAT4	() const { return { x, y, z, w }; }
	inline			operator string				() const { return string("{ ", x, ", ", y, ", ", z, ", ", w, " }"); }

	explicit inline operator xmvector() const { return DirectX::XMVectorSet(x, y, z, w); }

	SERIALIZE(x, y, z, w)
};

inline float	dot		(float4 a,			float4 b						)	{ return { a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w }; }
inline float4   clamp	(float4 min,		float4 x,			float4 max	)	{ return { clamp(min.x, x.x, max.x),	clamp(min.y, x.y, max.y),	clamp(min.z, x.z, max.z),	clamp(min.w, x.w, max.w) }; }
inline float4   wrap	(float4 min,		float4 x,			float4 max	)	{ return { wrap(min.x, x.x, max.x),		wrap(min.y, x.y, max.y),	wrap(min.z, x.z, max.z),	wrap(min.w, x.w, max.w) }; }
inline float4	lerp	(const float4& a,	const float4& b,	float t		)	{ return (1 - t) * a + t * b; }

inline float	dist	(const float4& a, const float4& b) { return (a - b).mag(); }