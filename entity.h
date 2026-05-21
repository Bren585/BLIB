#pragma once
#include "object.h"
#include "collider.h"

namespace BLIB {

	namespace generic {
		class entity : public renderable {
		protected:
			/* Collision Markers */
			bool on_floor = false;
			bool on_wall = false;
			bool on_ceil = false;

			virtual void _update(float elapsed_time) {}

		public:
			virtual ~entity() = default;

			/* Collision Markers */
			virtual void on_floored	() {}
			virtual void on_walled	() {}
			virtual void on_ceiled	() {}

			void floor	(bool state = true)	{ if (state && !on_floor	) { on_floored	();	} on_floor	= state; }
			void wall	(bool state = true)	{ if (state && !on_wall		) { on_walled	();	} on_wall	= state; }
			void ceil	(bool state = true)	{ if (state && !on_ceil		) { on_ceiled	();	} on_ceil	= state; }

			bool floored() const { return on_floor; }
			bool walled	() const { return on_wall;	}
			bool ceiled	() const { return on_ceil;	}

			/* Render */
			virtual void render(render_settings settings) const {}

			/* Update */
			virtual void update(float dt) = 0;

			/* Colliders */
			virtual void set_collider(collider* c) = 0;
			virtual const collider* peek_collider() const = 0;
			virtual collider* get_collider() = 0;
		};
	}

	namespace base {
		template <class V, class A> class entity : public generic::entity {
		protected:
			/* Globals */
			static inline V gravity = V{};
			static inline V tvel	= V{FLT_MAX};

			/* Physics */
			V		vel;
			A		avel;
			V		svel;
			float	weight;
			float	drag;

		public:
			virtual ~entity() = default;

			/* Globals */
			static void set_global_gravity				(V g) { gravity = g; }
			static void set_global_terminal_velocity	(V t) { tvel	= t; }

			static V get_global_gravity() { return gravity; }

			/* Constructors */
			entity() : vel(), avel(), svel(), weight(1), drag(0) {}
			entity(const entity<V, A>& e) : vel(e.vel), avel(e.avel), svel(e.svel), weight(e.weight), drag(e.drag) {}

			/* Getters + Setters */
			V		get_vel() const { return vel;		}
			V		get_svl() const { return svel;		}
			A		get_avl() const { return avel;		}
			float	get_wgt() const { return weight;	}
			float	get_drg() const { return drag;		}	

			void set_vel(V		v) { vel	= v; }
			void set_svl(V		s) { svel	= s; } 
			void set_avl(A		a) { avel	= a; }
			void set_wgt(float  w) { weight = w; }
			void set_drg(float	d) { drag	= clamp(0, d, 1); }

			void add_vel(V		d) { vel	+= d; }
			void add_svl(V		d) { svel	+= d; }
			void add_avl(A		d) { avel	+= d; }
			void add_wgt(float  d) { weight += d; }

			void mlt_vel(V		d) { vel	*= d; }
			void mlt_svl(V		d) { svel	*= d; }
			void mlt_avl(A		d) { avel	*= d; }
			void mlt_wgt(float  d) { weight *= d; }

#ifdef _DEBUG
			V&		imgui_get_vel() { return vel; }
			V&		imgui_get_svl() { return svel; }
			A&		imgui_get_avl() { return avel; }
			float&	imgui_get_wgt() { return weight; }
			float&	imgui_get_drg() { return drag; }
#endif

		};
	}

	namespace flat {
#define FLAT_BASE base::entity<float2, float>

		class entity : protected object, public FLAT_BASE {
		public:
			/* Constructors */
			entity()					: FLAT_BASE(),	object()		{}
			entity(string file)			: FLAT_BASE(),	object(file)	{}
			entity(const object& o)		: FLAT_BASE(),	object(o)		{}
			entity(const entity& e)		: FLAT_BASE(e),	object(e)		{}

			virtual ~entity() = default;

			entity operator=(const entity& e) { object::operator=(e); vel = e.vel; avel = e.avel; weight = e.weight; return e; }
			object operator=(const object& o) { return object::operator=(o); }

			operator object&() { return *this; }
			operator float2() { return pos; }
			
			/* Render */
			void render(render_settings settings = {}) const override { object::render(settings); }

			/* Update */
			inline virtual void _update(float dt) override { 
				vel = clamp(-tvel, (vel + (gravity * weight * dt)) * (1 - drag * dt), tvel); 
				pos += vel * dt; 
				//scale *= svel * dt;
				angle += avel * dt; 
			}

			void update(float elapsed_time) override { entity::_update(elapsed_time); object::update(elapsed_time); }

			/* Object Getters + Setters */

			float2	get_size	() const	{ return object::get_size();	}
			float2	get_pvt		() const	{ return pivot;					}
			float2	get_pos		() const	{ return pos;					}
			float2	get_scl		() const	{ return scale;					}
			float	get_ang		() const	{ return angle;					}

			void set_size	(float2 s)	{ object::set_size(s);	}
			void set_pvt	(float2 c)	{ pivot = c;			}
			void set_pos	(float2 p)	{ pos	= p;			}
			void set_scl	(float2 s)	{ scale = s;			}
			void set_ang	(float  a)	{ angle	= a;			}

			void add_pos(float2 d)	{ pos	+= d; }
			void add_scl(float2 d)	{ scale += d; }
			void add_ang(float  d)	{ angle	+= d; }

			void mlt_pos(float2 d)	{ pos	*= d; }
			void mlt_scl(float2 d)	{ scale *= d; }
			void mlt_ang(float  d)	{ angle	*= d; }

#ifdef _DEBUG
			float2& imgui_get_pos() { return pos; }
			float2& imgui_get_scl() { return scale; }
			float2& imgui_get_pvt() { return pivot; }
			float&	imgui_get_ang() { return angle; }
#endif

			void set_clr(color c)	{ tint = c; }

			void load_sprite(string filename)	{ object::load_sprite(filename);	}
			void set_sprite(sprite* spr)		{ object::set_sprite(spr);			}
			void make_dummy(color c)			{ object::make_dummy(c);			}

			void			set_collider	(generic::collider* c)	override { object::set_collider(c);			}
			const collider* peek_collider	() const				override { return object::peek_collider();	}
			collider*		get_collider	()						override { return object::get_collider();	}

			texture_animator& get_animator	()								 { return object::peek_animator();	}

			operator const object () const { return *this; }
			const object* obj() const { return this; }
		};

	}

	namespace full {
#define FULL_BASE base::entity<float3, float3>

		class entity : protected object, public FULL_BASE {
		public:
			/* Constructors */
			entity()				: FULL_BASE(),  object()  {}
			entity(const object& o) : FULL_BASE(),  object(o) {}
			entity(const entity& e) : FULL_BASE(e), object(e) {}

			virtual ~entity() = default;

			entity operator=(const entity& e) { object::operator=(e); vel = e.vel; avel = e.avel; weight = e.weight; return e; }
			object operator=(const object& o) { return object::operator=(o); }

			operator object& () { return *this; }
			operator float3() { return get_pos(); }

			/* Render */
			void render(render_settings settings = {}) const override { object::render(settings); }

			/* Update */
			inline virtual void _update(float dt) override { 
				vel = clamp(-tvel, (vel + (gravity * weight * dt)) * (1 - drag * dt), tvel);
				object::add_pos(vel * dt); 
				object::mlt_scl(((svel * dt) + float3(1.0f, 1.0f, 1.0f)));
				object::add_ang(avel * dt); 
			}

			void update(float elapsed_time) override { entity::_update(elapsed_time); object::update(elapsed_time); }

			/* Object Getters + Setters */
			void set_model(model* m) { object::set_model(m); }
			void copy_model(const model* m) { object::copy_model(m); }

			model*			get_model()			{ return object::get_model(); }
			const model*	get_model() const	{ return object::get_model(); }

			void			set_collider	(generic::collider* c)	override { object::set_collider(c);			}
			const collider* peek_collider	() const				override { return object::peek_collider();	}
			collider*		get_collider	()						override { return object::get_collider();	}

			float3 get_size() { return object::get_size(); }

			float3		get_pos() const { return object::get_pos(); }
			quaternion  get_qtn() const { return object::get_qtn(); }
			float3		get_scl() const { return object::get_scl(); }
			float3		get_pvt() const { return object::get_pvt(); }

#ifdef _DEBUG
			float3&		imgui_get_pos() { return object::imgui_get_pos(); }
			quaternion& imgui_get_qtn() { return object::imgui_get_qtn(); }
			float3&		imgui_get_scl() { return object::imgui_get_scl(); }
			float3&		imgui_get_pvt() { return object::imgui_get_pvt(); }

			void imgui_force_update() { object::imgui_force_update(); }
#endif

			void set_pos(float3		p) { object::set_pos(p); }
			void set_ang(float3		a) { object::set_ang(a); }
			void set_qtn(quaternion q) { object::set_qtn(q); }
			void set_scl(float3		s) { object::set_scl(s); }
			void set_pvt(float3		p) { object::set_pvt(p); }

			void add_pos(float3		d) { object::add_pos(d); }
			void add_ang(float3		d) { object::add_ang(d); }
			void add_scl(float3		d) { object::add_scl(d); }
			void add_pvt(float3		d) { object::add_pvt(d); }

			void mlt_pos(float3		d) { object::mlt_pos(d); }
			void mlt_qtn(quaternion	d) { object::mlt_qtn(d); }
			void mlt_scl(float3		d) { object::mlt_scl(d); }
			void mlt_pvt(float3		d) { object::mlt_pvt(d); }

			void set_clr(color c) { tint = c; }

			//bool ray_collision(const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const { return object::ray_collision(origin, ray, out_int_point, out_int_normal, any_hit); }
		
			operator const object() const { return *this; }
			const object* obj() const { return this; }
		};
	}
}
