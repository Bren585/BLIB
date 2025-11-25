#pragma once

#include "transform.h"

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
