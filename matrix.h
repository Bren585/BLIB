#pragma once

#include "float4.h"

/***********************************************************************************************************************
													matrix
************************************************************************************************************************/

typedef DirectX::XMMATRIX matrix;

inline matrix inverse3x3(const matrix& m) {
	matrix trunc = m;
	trunc.r[3] = DirectX::XMVectorSet(0, 0, 0, 1);
	return DirectX::XMMatrixInverse(nullptr, trunc);
}

inline matrix transpose(const matrix& m) {
	return DirectX::XMMatrixTranspose(m);
}

inline float3 mul(const float3& v, const matrix& m) { return DirectX::XMVector3Transform((xmvector)v, m); }
inline float4 mul(const float4& v, const matrix& m) { return DirectX::XMVector4Transform((xmvector)v, m); }

inline float3 mul_normal(const float3& v, const matrix& m) {				return		(float3)DirectX::XMVector3TransformNormal((xmvector)v,			m);			}
inline float4 mul_normal(const float4& v, const matrix& m) { float w = v.w;	return {	(float3)DirectX::XMVector3TransformNormal((xmvector)v.xyz(),	m), w };	}
