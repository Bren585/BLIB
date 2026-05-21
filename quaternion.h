#pragma once

#include "matrix.h"
#include "float3x3.h"

/***********************************************************************************************************************
													quaternion
************************************************************************************************************************/

class quaternion : public float4 {
public:
	quaternion()									: float4(0, 0, 0, 1)	{}
	quaternion(DirectX::XMFLOAT4 v)					: float4(v)				{}
	quaternion(float x, float y, float z, float w)	: float4(x, y, z, w)	{}
	quaternion(const float3& v, float w)			: float4(v, w)			{}
	quaternion(const float4& v)						: float4(v)				{}
	quaternion(const xmvector& v)					: float4(v)				{}
	
	static quaternion identity() { return quaternion(0, 0, 0, 1); }

	quaternion norm() const {
		bool degen = true;
		for (int i = 0; i < 4; i++) { if (!is_zero(operator[](i))) { degen = false; break; } }
		if (degen) return identity();
		else return operator/(mag());
	}

	quaternion(const float3x3& m) {
		const float3 right = m.right(), up = m.up(), forward = m.forward();
		float rx = right.x, uy = up.y, fz = forward.z;

		float trace = rx + uy + fz;
		if (trace > EPS) {
			float s = sqrtf(trace + 1.0f) * 2.0f;
			float inv = 1.0f / s;

			x = (forward.y	- up.z		) * inv;
			y = (right.z	- forward.x	) * inv;
			z = (up.x		- right.y	) * inv;
			w = 0.25f * s;
		}
		else if (rx > uy && rx > fz) {
			float s = sqrtf(1.0f + rx - uy - fz) * 2.0f;
			float invs = 1.0f / s;

			x = 0.25f * s;
			y = (right.y	+ up.x		) * invs;
			z = (right.z	+ forward.x	) * invs;
			w = (forward.y	- up.z		) * invs;
		}
		else if (uy > fz) {
			float s = sqrtf(1.0f + uy - rx - fz) * 2.0f;
			float invs = 1.0f / s;

			x = (right.y	+ up.x		) * invs;
			y = 0.25f * s;
			z = (up.z		+ forward.y	) * invs;
			w = (right.z	- forward.x	) * invs;
		}
		else {
			float s = sqrtf(1.0f + fz - rx - uy) * 2.0f;
			float invs = 1.0f / s;

			x = (right.z	+ forward.x	) * invs;
			y = (up.z		+ forward.y	) * invs;
			z = 0.25f * s;
			w = (up.x		- right.y	) * invs;
		}

		norm();
	}

	quaternion operator*(const float4& v) const {
		return quaternion(
			w * v.x + x * v.w + y * v.z - z * v.y,
			w * v.y - x * v.z + y * v.w + z * v.x,
			w * v.z + x * v.y - y * v.x + z * v.w,
			w * v.w - x * v.x - y * v.y - z * v.z
		);
	}

	quaternion operator*(const quaternion& q) const {
		return operator*(float4(q)).norm();
	}

	quaternion operator*=(const quaternion& q) { return *this = operator*(q); }

	quaternion operator*	(float s) const { return float4::operator*(s);	}
	quaternion operator*=	(float s)		{ return float4::operator*=(s);	}

	quaternion conj() const { return quaternion(-x, -y, -z, w); }

	quaternion inv() const {
		float mgsq = mag_sq();
		if (mgsq > EPS) return conj() * (1.0f / mgsq);
		else return identity();
	}

	float3 rotate(const float3& v) {
		if (!v) return v;
		return ((*this) * float4{ v, 0 } * float4(conj())).xyz();
	}

	static quaternion from_axis_angle(float3 axis, float angle) {
		axis = axis.norm();
		return quaternion{ axis * sinf(angle * 0.5f), cosf(angle * 0.5f) }.norm();
	}

	static quaternion from_euler(const float3& rpy) {
		float3 s = { sinf(rpy.x * 0.5f), sinf(rpy.y * 0.5f), sinf(rpy.z * 0.5f) };
		float3 c = { cosf(rpy.x * 0.5f), cosf(rpy.y * 0.5f), cosf(rpy.z * 0.5f) };

		return {
			s.x * c.y * c.z - c.x * s.y * s.z,
			c.x * s.y * c.z + s.x * c.y * s.z,
			c.x * c.y * s.z - s.x * s.y * c.z,
			c.x * c.y * c.z + s.x * s.y * s.z
		};
	}

	static quaternion face_to(const float3& local_vector, const float3& dest_vector) {
		float d = dot(local_vector, dest_vector);
		
		if (d > ONE_APPROX) {
			return identity();
		}

		if (d < -ONE_APPROX) {
			float3 axis = std::abs(local_vector.x) < 0.9f ? float3(1, 0, 0) : float3(0, 1, 0);
			axis = (local_vector % axis).norm();
			return from_axis_angle(axis, PI);
		}

		float3 axis = local_vector % dest_vector;
		if (axis) {
			axis.norm();
			float angle = acosf(dot(local_vector, dest_vector));
			if (non_zero(angle)) {
				return from_axis_angle(axis, angle);
			}
		}
		return identity();
	}

	operator float3x3() const {
		float	xx = x * x, yy = y * y, zz = z * z,
				xy = x * y, xz = x * z, yz = y * z,
				wx = w * x, wy = w * y, wz = w * z;

		return {
			{ 1 - 2 * (yy + zz),     2 * (xy + wz),     2 * (xz - wy) },
			{     2 * (xy - wz), 1 - 2 * (xx + zz),     2 * (yz + wx) },
			{     2 * (xz + wy),     2 * (yz - wx), 1 - 2 * (xx + yy) }
		};
	}

	float3 to_euler() {
		matrix M{ DirectX::XMMatrixRotationQuaternion(xmvector(*this)) };

		return {
			asinf(-M.r[2].m128_f32[1]),						// pitch
			atan2f(M.r[2].m128_f32[0], M.r[2].m128_f32[2]), // yaw
			atan2f(M.r[0].m128_f32[1], M.r[1].m128_f32[1])	// roll
		};
	}

	SERIALIZE(x, y, z, w)
};

inline quaternion slerp(const quaternion& a, quaternion b, float t) {
	float d = dot(a, b);
	
	if (d < 0.0f) {
		b = -b;
		d = -d;
	}

	if (d > ONE_APPROX) {
		return quaternion(a * (1 - t) + b * t).norm();
	}

	float theta = acosf(d);
	float s = sinf(theta);

	float w1 = sinf((1 - t) * theta) / s;
	float w2 = sinf(t * theta) / s;

	return (a * w1 + b * w2).norm();
}