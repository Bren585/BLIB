#pragma once

#include "DirectXMath.h"

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