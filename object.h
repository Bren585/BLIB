#pragma once
#include "interfaces.h"
#include "sprite.h"
#include "model.h"
#include "collider.h"
#include <memory>

namespace BLIB {

	namespace generic {
		class object : public renderable {
		private:
			render_settings default_settings;
		protected:
			std::unique_ptr<collider> collision;
			virtual void sync_collider() = 0;

			virtual void _basic_update(float elapsed_time) = 0;
			virtual void _render(render_settings settings) const = 0;

		public: // Inherit as Protected
			color tint;

		public: // Redefine as Public
			object(color c = WHITE) : tint(c) {}
			virtual ~object() = default;
			inline void render(render_settings settings = {}) const { _render(default_settings & settings); }

			inline void update(float elapsed_time) { _basic_update(elapsed_time); sync_collider(); }

			void set_settings(render_settings settings) { default_settings  = settings; }
			void add_settings(render_settings settings) { default_settings &= settings; }
			render_settings get_settings() const { return default_settings; }

			virtual void set_collider(collider* c) = 0;
			virtual const collider* peek_collider() const = 0;
			virtual collider* get_collider() = 0;

		};
	}

	namespace flat {

		class object : public generic::object {
		private:
			std::unique_ptr<sprite> spr;
			string filename;

		protected:
			void sync_collider() override { 
				static_cast<collider*>(collision.get())->sync(pos, scale, angle); 
			}
			
			virtual void _render(render_settings settings) const override { if (spr) { settings.set(); spr->render(bl(), size * scale, tpos, tsize, angle, pivot, tint); } }
			virtual void _basic_update(float elapsed_time) override {}

		public: // Inherit as Protected
			float2	pos;
			float2	size;
			float2	tpos;
			float2	tsize;
			float	angle;
			float2	scale;
			float2	pivot;

			inline float2 bl() const { return pos - (float2(1) + pivot) * 0.5f * size; }
			inline float2 tr() const { return pos + (float2(1) - pivot) * 0.5f * size; }

			sprite* peek_sprite() { return spr.get(); }

			object()				: spr(nullptr),														angle(0),		scale(1),		pivot(C_BL)																		{ set_settings({ pixel_shader("default_flat") }); }
			object(string file)		: spr(std::make_unique<sprite>(file)),	filename(file),				angle(0),		scale(1),		pivot(C_BL)																		{ set_settings({ pixel_shader("default_flat"), vertex_shader(sprite::get_default_vs()) }); tsize = size = spr->get_size(); }
			object(const object& o) : spr(o.spr->clone()),					filename(o.filename),		angle(o.angle), scale(o.scale),	pivot(o.pivot), pos(o.pos), size(o.size), tpos(o.tpos), generic::object(o.tint) { set_settings(o.get_settings()); }
			virtual ~object() = default;

			object operator=(const object& o) { spr = std::make_unique<sprite>(o.filename); filename = o.filename; pos = o.pos; size = o.size; tpos = o.tpos; tsize = o.tsize; angle = o.angle; pivot = o.pivot; tint = o.tint; return o; }

		public: // Redefine as Public after Protected Inheritence
			
			void load_sprite(string file)			{ filename = file;		spr = std::make_unique<sprite>(file);		tsize = size = spr->get_size();	add_settings(sprite::get_default_vs());		}
			void copy_sprite(sprite* old_sprite)	{ assert(old_sprite);	spr.reset(old_sprite->clone());				tsize = size = spr->get_size();	add_settings(sprite::get_default_vs());		}
			void set_sprite(sprite* new_sprite)		{						spr.reset(new_sprite);		if (spr)	  { tsize = size = spr->get_size();	add_settings(sprite::get_default_vs()); }	}
			void make_dummy(color c)				{						spr = std::make_unique<sprite>(c, 1.0f);	tsize = size = 1.0f;			add_settings(sprite::get_default_vs());		}

			inline float2 get_spr_size() const { if (spr) return spr->get_size(); else return 0; }

			void set_collider(generic::collider* c) override { collider* temp = dynamic_cast<collider*>(c); assert(temp); collision.reset(temp); }
			const collider* peek_collider() const override { return static_cast<collider*>(collision.get()); }
			collider* get_collider() override { return static_cast<collider*>(collision.get()); }
		};
	}

	namespace full {

		class object : public generic::object {
		private:
			std::unique_ptr<model>	mdl;
			transform				trans;

		protected:
			void sync_collider() override { static_cast<collider*>(collision.get())->sync(trans); }
			
			virtual void _render(render_settings settings) const override { if (mdl) { settings.set(); mdl->render(trans, tint); } }
			virtual void _basic_update(float elapsed_time) override { if (mdl) mdl->update(elapsed_time); }

		public: // Inherit as Protected
			object()				: mdl(nullptr)													{ set_settings({ pixel_shader("defualt_full") });	}
			object(const object& o) : mdl(o.mdl->clone()), trans(o.trans), generic::object(o.tint)	{ set_settings(o.get_settings());					}
			virtual ~object() = default;

			object& operator=(const object& o) { mdl = (o.mdl ? o.mdl->clone() : nullptr); trans = o.trans; tint = o.tint; return *this; }

		public: // Redefine as Public after Protected Inheritence

			model*			get_model()						{ return mdl.get(); }
			const model*	get_model() const				{ return mdl.get(); }
			void			set_model(model* m)				{ mdl.reset(m);		add_settings({ vertex_shader("default_full") }); }
			void			copy_model(const model* m)		{ mdl = m->clone(); add_settings({ vertex_shader("default_full") }); }

			transform		get_trans()	const				{ return trans; }
			void			set_trans(const transform& t)	{ trans = t; }

			virtual float3	get_size() const { if (mdl) return mdl->get_size() * trans.get_scl(); else return 0; }

			void set_collider(generic::collider* c) override { collider* temp = dynamic_cast<collider*>(c); assert(temp); collision.reset(temp); }
			const collider* peek_collider() const override { return static_cast<collider*>(collision.get()); }
			collider* get_collider() override { return static_cast<collider*>(collision.get()); }

			float3 get_mid() const { return trans.get_mid(); }
			float3 get_pos() const { return trans.get_pos(); }
			float3 get_ang() const { return trans.get_ang(); }
			float3 get_scl() const { return trans.get_scl(); }
			float3 get_pvt() const { return trans.get_pvt() / get_size() * 2.0f; } // convert clip space to world space

#ifdef _DEBUG
			float3& imgui_get_pos() { return trans.get_pos_ref(); }
			float3& imgui_get_ang() { return trans.get_ang_ref(); }
			float3& imgui_get_scl() { return trans.get_scl_ref(); }
			float3& imgui_get_pvt() { return trans.get_pvt_ref(); }

			void imgui_force_update() { trans.force_update(); }
#endif

			void set_pos(float3 p) { trans.set_pos(p); }
			void set_ang(float3 a) { trans.set_ang(a); }
			void set_scl(float3 s) { trans.set_scl(s); }
			void set_pvt(float3 p) { trans.set_pvt(p * get_size() * 0.5f); } // convert clip space to world space

			void add_pos(float3 d) { trans.add_pos(d); }
			void add_ang(float3 d) { trans.add_ang(d); }
			void add_scl(float3 d) { trans.add_scl(d); }
			void add_pvt(float3 p) { trans.add_pvt(p * get_size() * 0.5f); } // convert clip space to world space

			void mlt_pos(float3 d) { trans.mlt_pos(d); }
			void mlt_ang(float3 d) { trans.mlt_ang(d); }
			void mlt_scl(float3 d) { trans.mlt_scl(d); }
			void mlt_pvt(float3 p) { trans.mlt_pvt(p * get_size() * 0.5f); } // convert clip space to world space

			//bool ray_collision(const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const { return mdl->ray_collision(trans, origin, ray, out_int_point, out_int_normal, any_hit); }
		};

	}
}