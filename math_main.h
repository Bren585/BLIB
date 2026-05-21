#pragma once

#include "DirectXMath.h"

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
#define PID2		DirectX::XM_PIDIV2
#define EPS			1e-6f
#define ZERO_APPROX	0.0001f
#define ONE_APPROX	0.9999f
#define VECTOR_UP	DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)

typedef unsigned int		uint;
typedef DirectX::XMVECTOR	xmvector;

/***********************************************************************************************************************
													functions
************************************************************************************************************************/

#define RADIANS(a)		DirectX::XMConvertToRadians(a)
#define DEGREES(a)		DirectX::XMConvertToDegrees(a)

#define TRANS_M(f3)		DirectX::XMMatrixTranslation(f3.x, f3.y, f3.z)
#define ROTATE_M(f3)	DirectX::XMMatrixRotationRollPitchYaw(f3.x, f3.y, f3.z)
#define QUAT_M(q)		DirectX::XMMatrixRotationQuaternion(xmvector(q))
#define SCALE_M(f3)		DirectX::XMMatrixScaling(f3.x, f3.y, f3.z)

ARITHMETIC inline	A		maxim(A a, A b) { return (a > b) ? a : b; }
ARITHMETIC inline	A		minim(A a, A b) { return (a < b) ? a : b; }

inline				float	clamp(float min, float x, float max) { return (x < min) ? min : ((x < max) ? x : max); }
ARITHMETIC inline	A		clamp(A min, A x, A max) { return (x < min) ? min : ((x < max) ? x : max); }
ARITHMETIC inline	A		wrap(A min, A x, A max) { return (x < min) ? x + max - min : ((x < max) ? x : x - max + min); }
ARITHMETIC inline	A		lerp(const A& a, const A& b, A t) { return (1 - t) * a + t * b; }
inline				bool	is_zero(float x) { return fabs(x) < ZERO_APPROX; }
inline				bool	non_zero(float x) { return fabs(x) > ZERO_APPROX; }