#pragma once

#include "float3.h"

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

	static float3x3 identity() {
		return {
			{ 1, 0, 0 },
			{ 0, 1, 0 },
			{ 0, 0, 1 }
		};
	}

	float3&			operator[](int i)		{ return r[i]; }
	const float3&	operator[](int i) const { return r[i]; }

	float3 x()			const { return { r[0].x, r[1].x, r[2].x }; }
	float3 y()			const { return { r[0].y, r[1].y, r[2].y }; }
	float3 z()			const { return { r[0].z, r[1].z, r[2].z }; }

	float3 right()		const { return r[0]; }
	float3 up()			const { return r[1]; }
	float3 forward()	const { return r[2]; }

	float3x3 transpose() const {
		float3x3 t;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				t.r[i][j] = r[j][i];
			}
		}
		return t;
	}

	float3 rotate		(const float3& t) const { return { dot(t, r[0]), dot(t, r[1]), dot(t, r[2]) }; }
	float3 inv_rotate	(const float3& t) const { return transpose().rotate(t); }

	float3 to_euler() const {
		float roll, pitch, yaw;

		float sy = clamp(-1.0f, -r[0].z, 1.0f);
		float cy = sqrtf(1.0f - sy * sy);
		pitch = atan2(sy, cy);

		if (fabsf(sy) < ONE_APPROX) {
			roll = -atan2(r[1].z / cy, r[2].z / cy);
			yaw = atan2(r[0].y / cy, r[0].x / cy);
		}
		else { // gimbal lock
			roll = 0.0f;
			yaw = atan2(-r[1].x, r[1].y);
		}

		return { roll, pitch, yaw };
	}
};

//inline float3x3 face_to(const float3& f) {
//	const float3 F = f.norm();
//	float3 R, U, H = { 0, 1, 0 }; // U Hint
//	if (fabsf(dot(H, F)) > ONE_APPROX) { H = { 1, 0, 0 }; } // R Hint
//	R = (H % F).norm();
//	U = (F % R).norm();
//	return { R, U, -F };
//}
//
//inline float3x3 head_to(const float3& u) {
//	const float3 U = u.norm();
//	float3 R, F, H = { 0, 0, -1 }; // F Hint
//	if (fabsf(dot(H, U)) > ONE_APPROX) { H = { 1, 0, 0 }; } // R Hint 
//	R = (H % U).norm();
//	F = (U % R).norm();
//	return { R, U, -F };
//}
//
//inline float3x3 side_to(const float3& r) {
//	const float3 R = r.norm();
//	float3 F, U, H = { 0, 1, 0 }; // U Hint
//	if (fabsf(dot(H, R)) > ONE_APPROX) { H = { 0, 0, -1 }; } // F Hint
//	F = (H % R).norm();
//	U = (R % F).norm();
//	return { R, U, -F };
//}