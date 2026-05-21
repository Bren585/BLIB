#pragma once
#include "math.h"
#include "model.h"
#include "hierarchy.h"
#include "all_shapes.h"

// MESH COLLISIONS ARE DISABLED FOR PERFORMANCE, RAY PICKING IS AVAILABLE

#define NO_COLLISION_2 {-1, {0, 0}}
#define NO_COLLISION_3 {-1, {0, 0, 0}}

#define NO_HIT		FLT_MAX
#define ANY_HIT		1.0f
#define IS_HIT(x)	x != NO_HIT

#define DEFAULT_FRICTION	0.1f
#define DEFAULT_RESTITUTION 0.2f

namespace BLIB::flat { class collider; struct collision; class object; class entity; }
namespace BLIB::full { class collider; struct collision; class object; class entity; }

namespace BLIB::collision {

	enum collider_type {
		none,  // Does not cause collision
		bully, // Will never move
		loser, // Will always move
		swing  // Will lose to a bully but win against a loser
	};

	enum collision_type {
		no_move,	// Neither will move
		a_moves,	// Only "a" will move
		b_moves,	// Only "b" will move
		compromise	// Both will move
	};

	collision_type resolve(collider_type a, collider_type b);

	flat::collision check(const flat::collider* a, const flat::collider* b, bool any = false);
	full::collision check(const full::collider* a, const full::collider* b, bool any = false);

	flat::collision check(flat::object* a, flat::object* b, bool do_move = true);
	flat::collision check(flat::entity* a, flat::entity* b, bool do_move = true);
	flat::collision check(flat::entity* a, flat::object* b, bool do_move = true);
	flat::collision check(flat::object* a, flat::entity* b, bool do_move = true);

	full::collision check(full::object* a, full::object* b, bool do_move = true);
	full::collision check(full::entity* a, full::entity* b, bool do_move = true);
	full::collision check(full::entity* a, full::object* b, bool do_move = true);
	full::collision check(full::object* a, full::entity* b, bool do_move = true);

	void move(flat::object* obj, flat::object* mover, flat::collision penetration, bool both = false);
	void move(flat::entity* obj, flat::object* mover, flat::collision penetration, bool both = false);
	void move(flat::entity* obj, flat::entity* mover, flat::collision penetration, bool both = false);

	void move(full::object* obj, full::object* mover, full::collision penetration, bool both = false);
	void move(full::entity* obj, full::object* mover, full::collision penetration, bool both = false);
	void move(full::entity* obj, full::entity* mover, full::collision penetration, bool both = false);

	void land(flat::entity* obj, const flat::collision& penetration);
	void land(full::entity* obj, const full::collision& penetration);

	bool ray_pick(const flat::object* obj, const float2& ray_origin, const float2& ray_direction, float2* out_position = nullptr, float2* out_normal = nullptr);
	bool ray_pick(const flat::entity* obj, const float2& ray_origin, const float2& ray_direction, float2* out_position = nullptr, float2* out_normal = nullptr);
	bool ray_pick(const full::object* obj, const float3& ray_origin, const float3& ray_direction, float3* out_position = nullptr, float3* out_normal = nullptr);
	bool ray_pick(const full::entity* obj, const float3& ray_origin, const float3& ray_direction, float3* out_position = nullptr, float3* out_normal = nullptr);
}

// Generic

namespace BLIB::generic {

	using namespace BLIB::collision;

	class collider : public hierarchy<collider> {
	private:
		collider_type type = none;
		float restitution	= DEFAULT_RESTITUTION;
		float friction		= DEFAULT_FRICTION;

	protected:
		virtual collider* clone_impl(collider* parent) const = 0;

		void weak_clone(collider* parent) const {
			collider* out = clone_impl(parent);
			for (auto& child : children) { child->weak_clone(out); }
		}
		
		virtual void _render_debug(render_settings rs) const {}

	public:
		collider(collider* parent) : hierarchy(parent) {}
		collider(const collider& o, collider* new_parent) : hierarchy(new_parent), type(o.type), restitution(o.restitution), friction(o.friction) {}

		void	set_type		(collider_type t)	{ type			= t; }
		void	set_restitution	(float v)			{ restitution	= v; }
		void	set_friction	(float v)			{ friction		= v; }

		collider_type	get_type		() const { return type;			}
		float			get_restitution	() const { return restitution;	}
		float			get_friction	() const { return friction;		}

#ifdef _DEBUG
		float&			imgui_get_restitution	() { return restitution;	}
		float&			imgui_get_friction		() { return friction;		}
#endif

		std::unique_ptr<collider> clone() const { 
			auto out = std::unique_ptr<collider>(clone_impl(nullptr)); 
			for (auto& child : children) { child->weak_clone(out.get()); }
			return out;
		};

		void render_debug(render_settings rs) const {
#ifdef _DEBUG
			_render_debug(rs);
#endif
		}
	};

}

// 2D Colliders

namespace BLIB::flat {

	struct collision {
		float depth;
		float2 normal;
		float restitution	= 0;
		float friction		= 0;

		collision(float d, float2 n) : depth(d), normal(n) {}
		collision(bool success) : depth(-1) { assert(!success); }

		collision operator-() const { return { depth, -normal }; }
		operator bool() { return (depth > 0); }
	};

	class aligned_rect_collider;
	class rect_collider;
	class circle_collider;

	class collider : public generic::collider {
	protected:
		float2 offset;
		float2 scale{1};

		float2 parent_position;
		float2 parent_scale;
		float  parent_angle = 0;

		virtual collider* clone_impl(generic::collider* parent) const override = 0;
		
		virtual float _ray_pick(float2 ray_origin, float2 ray_direction, float2* out_position = nullptr, float2* out_normal = nullptr) const = 0;

	public:
		collider(collider* parent) : generic::collider(parent) {}
		collider(const collider& o, collider* new_parent) : generic::collider(o, new_parent), offset(o.offset), scale(o.scale) {}

		virtual void sync(float2 p, float2 s, float a) { parent_position = p + offset.rotate(a); parent_scale = s; parent_angle = a; for (auto& child : children) { static_cast<collider*>(child.get())->sync(parent_position, parent_scale, parent_angle); } }

		void set_off(float2 o) { offset = o; }
		void add_off(float2 o) { offset += o; }

		void set_scl(float2 s) { scale = s; }
		void mlt_scl(float2 s) { scale *= s; }
		float2 get_scl() const { return scale * parent_scale; }

		float2 get_pos() const { return parent_position; }

		virtual collision collide(const collider* o) const = 0;

		virtual collision collide_with(const aligned_rect_collider*	o) const = 0;
		virtual collision collide_with(const rect_collider*			o) const = 0;
		virtual collision collide_with(const circle_collider*		o) const = 0;

		float ray_pick(float2 ray_origin, float2 ray_direction, float2* out_position = nullptr, float2* out_normal = nullptr) const;
	};

	class aligned_rect_collider : public collider {
	private:
		float2 size;

		aligned_rect_collider* clone_impl(generic::collider* parent) const override { return new aligned_rect_collider(*this, static_cast<collider*>(parent)); }

		float _ray_pick(float2 ray_origin, float2 ray_direction, float2* out_position = nullptr, float2* out_normal = nullptr) const override;
		
		void _render_debug(render_settings rs) const override;
	
	public:
		aligned_rect_collider(collider* parent, float2 size) : collider(parent), size(size) {}
		aligned_rect_collider(const aligned_rect_collider& o, collider* new_parent) : collider(o, new_parent), size(o.size) {}

		float2 get_size() const { return size * get_scl(); }

		collision collide(const collider* o) const override { return o->collide_with(this); };

		collision collide_with(const aligned_rect_collider*	o) const override;

		collision collide_with(const rect_collider*			o) const override;
		collision collide_with(const circle_collider*		o) const override;
	};

	class rect_collider : public collider {
	private:
		float2 size;
		float angle;

		rect_collider* clone_impl(generic::collider* parent) const override { return new rect_collider(*this, static_cast<collider*>(parent)); }

		float _ray_pick(float2 ray_origin, float2 ray_direction, float2* out_position = nullptr, float2* out_normal = nullptr) const override;
		
		void _render_debug(render_settings rs) const override;

	public:
		rect_collider(collider* parent, float2 size, float angle) : collider(parent), size(size), angle(angle) {}
		rect_collider(const rect_collider& o, collider* new_parent) : collider(o, new_parent), size(o.size), angle(o.angle) {}

		float2 get_size() const { return size * get_scl(); }
		float  get_ang() const { return angle + parent_angle; }

		collision collide(const collider* o) const override { return o->collide_with(this); };

		collision collide_with(const rect_collider*			o) const override;

		collision collide_with(const aligned_rect_collider*	o) const override;
		collision collide_with(const circle_collider*		o) const override;
	};

	class circle_collider : public collider {
	private:
		float r;

		circle_collider* clone_impl(generic::collider* parent) const override { return new circle_collider(*this, static_cast<collider*>(parent)); }

		float _ray_pick(float2 ray_origin, float2 ray_direction, float2* out_position = nullptr, float2* out_normal = nullptr) const override;

		void _render_debug(render_settings rs) const override;

	public:
		circle_collider(collider* parent, float r) : collider(parent), r(r) {}
		circle_collider(const circle_collider& o, collider* new_parent) : collider(o, new_parent), r(o.r) {}

		float get_r() const { return r * get_scl().x; }

		collision collide(const collider* o) const override { return o->collide_with(this); };

		collision collide_with(const circle_collider*		o) const override;

		collision collide_with(const aligned_rect_collider*	o) const override;
		collision collide_with(const rect_collider*			o) const override;
	};

}

// 3D Colliders

namespace BLIB::full {

	struct collision {
		float depth;
		float3 normal;
		float restitution	= 0;
		float friction		= 0;

		collision(float d, float3 n) : depth(d), normal(n) {}
		collision(bool success) : depth(-1) { assert(!success); }

		collision operator-() const { return { depth, -normal }; }
		operator bool() { return depth > 0; }
	};

	class aabb_collider;
	class box_collider;
	class sphere_collider;
	class cylinder_collider;
	class capsule_collider;
	class mesh_collider;
	class plane_collider;

	class collider : public generic::collider {
	protected:
		float3		offset;
		float3		scale{ 1 };
		transform	trans;

		mutable std::unique_ptr<model> debug_model = nullptr;

		virtual collider* clone_impl(generic::collider* parent) const override = 0;

		virtual float _ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const = 0;

	public:
		collider(collider* parent = nullptr) : generic::collider(parent) {}
		collider(const collider& o, collider* new_parent) : generic::collider(o, new_parent), offset(o.offset), scale(o.scale), trans(o.trans) {}

		virtual void sync(const transform& t) { 
			float3 pivot = trans.get_pvt();
			trans = t; 
			trans.set_pvt(pivot); 
			float3 rotated_offset = trans.get_qtn().rotate(offset);
			trans.add_pos(rotated_offset); 
			for (auto& child : children) { static_cast<collider*>(child.get())->sync(trans); } 
		}
		const transform& get_trans() const { return trans; }

		void set_pvt(float3 p) { trans.set_pvt(p); }

		void set_off(float3 o) { offset = o; }
		void add_off(float3 o) { offset += o; }

		void set_scl(float3 s) { scale = s; }
		void mlt_scl(float3 s) { scale *= s; }
		float3 get_scl() const { return scale * trans.get_scl(); }

		virtual operator aabb_collider() const = 0;

		virtual collision collide(const collider* o) const = 0;

		virtual collision collide_with(const aabb_collider*		o) const = 0;
		virtual collision collide_with(const box_collider*		o) const = 0;
		virtual collision collide_with(const sphere_collider*	o) const = 0;
		virtual collision collide_with(const cylinder_collider*	o) const = 0;
		virtual collision collide_with(const capsule_collider*	o) const = 0;
		virtual collision collide_with(const mesh_collider*		o) const = 0;
		virtual collision collide_with(const plane_collider*	o) const = 0;

		float ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position = nullptr, float3* out_normal = nullptr) const;

	};

	class aabb_collider : public collider {
	private:
		float3 size;

		aabb_collider* clone_impl(generic::collider* parent) const override { return new aabb_collider(*this, static_cast<collider*>(parent)); }

		float _ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const override;

		void _render_debug(render_settings rs) const override;

	public:
		aabb_collider(collider* parent, float3 size) : collider(parent), size(size) {}
		aabb_collider(const aabb_collider& o, collider* new_parent) : collider(o, new_parent), size(o.size) {}

		float3 get_size() const { return size * get_scl(); }

		operator aabb_collider() const { return aabb_collider(nullptr, size); }

		collision collide(const collider* o) const override { return o->collide_with(this); };

		collision collide_with(const aabb_collider*		o) const override;
		collision collide_with(const box_collider*		o) const override;
		collision collide_with(const sphere_collider*	o) const override;
		collision collide_with(const cylinder_collider*	o) const override;
		collision collide_with(const capsule_collider*	o) const override;
		collision collide_with(const mesh_collider*		o) const override;
		collision collide_with(const plane_collider*	o) const override;
	};

	class box_collider : public collider {
	private:
		float3 size;
		quaternion quat;

		box_collider* clone_impl(generic::collider* parent) const override { return new box_collider(*this, static_cast<collider*>(parent)); }

		float _ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const override;

		void _render_debug(render_settings rs) const override;

	public:
		box_collider(collider* parent, float3 size, quaternion quat) : collider(parent), size(size), quat(quat) {}
		box_collider(const box_collider& o, collider* new_parent) : collider(o, new_parent), size(o.size), quat(o.quat) {}

		float3		get_size()			const { return size * get_scl(); }
		quaternion	get_quaternion()	const { return quat * trans.get_qtn(); }

		operator aabb_collider() const;

		collision collide(const collider* o) const override { return o->collide_with(this); };

		collision collide_with(const aabb_collider*		o) const override;
		collision collide_with(const box_collider*		o) const override;
		collision collide_with(const sphere_collider*	o) const override;
		collision collide_with(const cylinder_collider*	o) const override;
		collision collide_with(const capsule_collider*	o) const override;
		collision collide_with(const mesh_collider*		o) const override;
		collision collide_with(const plane_collider*	o) const override;
	};

	class sphere_collider : public collider {
	private:
		float r;

		sphere_collider* clone_impl(generic::collider* parent) const override { return new sphere_collider(*this, static_cast<collider*>(parent)); }

		float _ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const override;

		void _render_debug(render_settings rs) const override;
	
	public:
		sphere_collider(collider* parent, float r) : collider(parent), r(r) {}
		sphere_collider(const sphere_collider& o, collider* new_parent) : collider(o, new_parent), r(o.r) {}

		float get_r() const { return r * get_scl().x; }

		operator aabb_collider() const;

		collision collide(const collider* o) const override { return o->collide_with(this); };

		collision collide_with(const aabb_collider*		o) const override;
		collision collide_with(const box_collider*		o) const override;
		collision collide_with(const sphere_collider*	o) const override;
		collision collide_with(const cylinder_collider*	o) const override;
		collision collide_with(const capsule_collider*	o) const override;
		collision collide_with(const mesh_collider*		o) const override;
		collision collide_with(const plane_collider*	o) const override;
	};

	class cylinder_collider : public collider {
	private:
		float r;
		float h;

		cylinder_collider* clone_impl(generic::collider* parent) const override { return new cylinder_collider(*this, static_cast<collider*>(parent)); }

		float _ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const override;

		void _render_debug(render_settings rs) const override;
	
	public:
		cylinder_collider(collider* parent, float r, float h) : collider(parent), r(r), h(h) {}
		cylinder_collider(const cylinder_collider& o, collider* new_parent) : collider(o, new_parent), r(o.r), h(o.h) {}

		float get_r() const { return r * get_scl().x; }
		float get_h() const { return h * get_scl().y; }

		operator aabb_collider() const;

		collision collide(const collider* o) const override { return o->collide_with(this); };

		collision collide_with(const aabb_collider*		o) const override;
		collision collide_with(const box_collider*		o) const override;
		collision collide_with(const sphere_collider*	o) const override;
		collision collide_with(const cylinder_collider*	o) const override;
		collision collide_with(const capsule_collider*	o) const override;
		collision collide_with(const mesh_collider*		o) const override;
		collision collide_with(const plane_collider*	o) const override;
	};

	class capsule_collider : public collider {
	private:
		float r;
		float h;

		capsule_collider* clone_impl(generic::collider* parent) const override { return new capsule_collider(*this, static_cast<collider*>(parent)); }

		float _ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const override;

		void _render_debug(render_settings rs) const override;

	public:
		capsule_collider(collider* parent, float r, float h) : collider(parent), r(r), h(h) {}
		capsule_collider(const capsule_collider& o, collider* new_parent) : collider(o, new_parent), r(o.r), h(o.h) {}

		float get_r() const { return r * get_scl().x; }
		float get_h() const { return h * get_scl().y; }

		operator aabb_collider() const;

		collision collide(const collider* o) const override { return o->collide_with(this); };

		collision collide_with(const aabb_collider*		o) const override;
		collision collide_with(const box_collider*		o) const override;
		collision collide_with(const sphere_collider*	o) const override;
		collision collide_with(const cylinder_collider*	o) const override;
		collision collide_with(const capsule_collider*	o) const override;
		collision collide_with(const mesh_collider*		o) const override;
		collision collide_with(const plane_collider*	o) const override;
	};

	class mesh_collider : public collider {
	private:
		const model* model_ptr;

		mesh_collider* clone_impl(generic::collider* parent) const override { return new mesh_collider(*this, static_cast<collider*>(parent)); }
	
		float _ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const override;

		void _render_debug(render_settings rs) const override;

	public:
		mesh_collider(collider* parent, const model* ptr) : collider(parent), model_ptr(ptr) {}
		mesh_collider(const mesh_collider& o, collider* new_parent) : collider(o, new_parent), model_ptr(o.model_ptr) {}

		const model* get_mesh() const { return model_ptr; }

		operator aabb_collider() const;

		collision collide(const collider* o) const override { return o->collide_with(this); };

		collision collide_with(const aabb_collider*		o) const override;
		collision collide_with(const box_collider*		o) const override;
		collision collide_with(const sphere_collider*	o) const override;
		collision collide_with(const cylinder_collider*	o) const override;
		collision collide_with(const capsule_collider*	o) const override;
		collision collide_with(const mesh_collider*		o) const override;
		collision collide_with(const plane_collider*	o) const override;
	};

	class plane_collider : public collider {
	private:
		float3 normal;

		plane_collider* clone_impl(generic::collider* parent) const override { return new plane_collider(*this, static_cast<collider*>(parent)); }

		float _ray_pick(float3 ray_origin, float3 ray_direction, float3* out_position, float3* out_normal) const override;

		void _render_debug(render_settings rs) const override;

	public:
		plane_collider(collider* parent, float3 n) : collider(parent), normal(n) {}
		plane_collider(const plane_collider& o, collider* new_parent) : collider(o, new_parent), normal(o.normal) {}

		float3 get_normal() const { return trans.get_qtn().rotate(normal); }

		operator aabb_collider() const { return aabb_collider(nullptr, float3{ FLT_MAX }); } // always collide

		collision collide(const collider* o) const override { return o->collide_with(this); };

		collision collide_with(const aabb_collider*		o) const override;
		collision collide_with(const box_collider*		o) const override;
		collision collide_with(const sphere_collider*	o) const override;
		collision collide_with(const cylinder_collider*	o) const override;
		collision collide_with(const capsule_collider*	o) const override;
		collision collide_with(const mesh_collider*		o) const override;
		collision collide_with(const plane_collider*	o) const override;
	};

}