#pragma once

#include "quaternion.h"
#include "float4x4.h"

/***********************************************************************************************************************
													transform
************************************************************************************************************************/

class transform {
private:
	mutable matrix mat;
	mutable bool needs_update = true;

	//if neccesarry
	//float4x4 f44;

	float3		pos	 ;
	float3		scale;
	quaternion	quat ;
	float3		pivot;
	 
public:
	transform(const matrix& decomp) : mat(MATRIX_ID) {
		xmvector p, s, q;
		DirectX::XMMatrixDecompose(&s, &q, &p, decomp);
		pos = p;
		scale = s;
		quat = q;
	}

	transform(const float4x4& decomp) : transform(DirectX::XMLoadFloat4x4(&decomp)) {}

#ifdef _DEBUG

	float3&		get_pos_ref() { return pos; }
	float3&		get_scl_ref() { return scale; }
	quaternion& get_qtn_ref() { return quat; }
	float3&		get_pvt_ref() { return pivot; }

#endif // _DEBUG

	float3		get_mid() const { return pos; }
	float3		get_pos() const { return pos + pivot; }
	float3		get_scl() const { return scale; }
	quaternion	get_qtn() const { return quat; }
	float3		get_pvt() const { return pivot; }

	float3x3 get_rot_mat() const { return quat; }

	void set_pos(float3		x) { pos	= x - pivot;					needs_update = true; }
	void set_scl(float3		x) { scale	= x;							needs_update = true; }
	void set_ang(float3		x) { quat	= quaternion::from_euler(x);	needs_update = true; }	
	void set_qtn(quaternion x) { quat	= x;							needs_update = true; }
	void set_pvt(float3		x) { pivot	= x;							needs_update = true; }

	void add_pos(float3		x) { pos	+= x;							needs_update = true; }
	void add_scl(float3		x) { scale	+= x;							needs_update = true; }
	void add_ang(float3		x) { quat	*= quaternion::from_euler(x);	needs_update = true; }
	void add_pvt(float3		x) { pivot	+= x;							needs_update = true; }

	void mlt_pos(float3		x) { pos	*= x;							needs_update = true; }
	void mlt_scl(float3		x) { scale	*= x;							needs_update = true; }
	void mlt_qtn(quaternion x) { quat	*= x;							needs_update = true; }
	void mlt_pvt(float3		x) { pivot	*= x;							needs_update = true; }

	void force_update() const { needs_update = true; }

	transform(float3 p = float3{0}, float3 s = float3{1}, quaternion q = quaternion::identity()) : pos(p), scale(s), quat(q), pivot(0), mat() {}
	transform(float x, float y, float z) : transform(float3(x, y, z)) {}

	inline void update_matrix() const { if (needs_update) { mat = TRANS_M(-pivot) * SCALE_M(scale) * QUAT_M(quat) * TRANS_M((pos + pivot)); needs_update = false; } }
	//inline void update_float() { update_matrix(); DirectX::XMStoreFloat4x4(&f44, mat); }

	inline operator matrix		() const { update_matrix(); return mat; }
	inline operator matrix&		() const { update_matrix(); return mat; }
	inline operator matrix*		() const { update_matrix(); return &mat; }

	inline transform operator+  (const transform& o) { return { pos + o.pos, scale * o.scale, quat * o.quat }; }
	inline transform operator+= (const transform& o) { *this = operator+(o); needs_update = true; return *this; }

	inline matrix operator* (transform& o)		{ return (matrix)*this * (matrix)o; }
	inline matrix operator* (const matrix& m)	{ return (matrix)*this * m; }

	inline operator float4x4		()				const { update_matrix(); float4x4 out;	DirectX::XMStoreFloat4x4(&out, mat); return out;	}
	inline void		get_float4x4	(float4x4& out) const { update_matrix();				DirectX::XMStoreFloat4x4(&out, mat);				}
	//inline operator float4x4& () { update_float(); return f44; } 

	SERIALIZE(pos, scale, quat, pivot)
};

inline transform lerp(const transform& a, const transform& b, float t) { 
	assert(a.get_pvt() == b.get_pvt());
	transform o; 
	o.set_pvt(a.get_pvt());
	o.set_pos(lerp(a.get_pos(), b.get_pos(), t)); 
	o.set_scl(lerp(a.get_scl(), b.get_scl(), t));
	o.set_qtn(slerp(a.get_qtn(), b.get_qtn(), t));
	return o; 
}

inline matrix lerp(const matrix& a, const matrix& b, float t) {
	return (matrix)lerp(transform(a), transform(b), t);
}

inline float4x4 lerp(const float4x4& a, const float4x4& b, float t) {
	return (float4x4)lerp(transform(a), transform(b), t);
}

inline transform position_transform		(float3 position) { return transform(position,	float3{1},	quaternion::identity()			); }
inline transform scale_transform		(float3 scale	) { return transform(float3{0},	scale,		quaternion::identity()			); }
inline transform rotation_transform		(float3 rotation) { return transform(float3{0},	float3{1},	quaternion::from_euler(rotation)); }
inline transform quaternion_transform	(quaternion quat) { return transform(float3{0}, float3{1},	quat							); }