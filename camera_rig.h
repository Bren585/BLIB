#pragma once
#include "camera.h"
#include "object.h"

namespace BLIB {
	namespace generic {
		class camera_rig {
		protected:
			camera* cam;
			virtual void sync_anchor() = 0;
			virtual void sync_rig	() = 0;

		protected:

			void sync() { sync_rig(); sync_anchor(); }

#ifdef _DEBUG
			mutable std::unique_ptr<generic::object> debug_obj = nullptr;
			virtual void _init_debug() = 0;
			virtual void _sync_debug() = 0;
#endif

		public:
			camera_rig() {}
			virtual ~camera_rig() {}

			void render_debug() {
#ifdef _DEBUG
				if (!debug_obj) { _init_debug(); }
				_sync_debug();
				debug_obj->render();
#endif
			}

			void update() { if (cam) { sync(); } };
			camera* get_camera() { return cam; }
			void set_camera(camera* c) { cam = c; }
		};
	}

	namespace flat {
		class camera_rig : public generic::camera_rig {
		private:
			void sync_anchor() override { cam->set_eye({ anchor->pos + offset_pos, -1}); }
		protected:
			const object* anchor;
			float2 offset_pos;
			float offset_angle;

#ifdef _DEBUG 
			void _init_debug() override {
				std::unique_ptr<flat::object> debug_spr = std::make_unique<flat::object>();
				debug_spr->make_dummy(WHITE);
				debug_spr->set_size(float2{ 25 });
				debug_obj = std::move(debug_spr);
			}

			void _sync_debug() override {
				flat::object* debug_spr = static_cast<flat::object*>(debug_obj.get());
				debug_spr->pos = anchor->pos + offset_pos;
				debug_spr->angle = anchor->angle + offset_angle;
			}
#endif

		public:
			camera_rig(const object* anchor, float2 offset = float2{ 0 }, float offset_angle = 0) : anchor(anchor), offset_pos(offset), offset_angle(offset_angle) {}
			virtual ~camera_rig() {}

			float2	get_offset()	const	{ return offset_pos; }
			void	set_offset(float2 p)	{ offset_pos = p; }
			void	add_offset(float2 d)	{ offset_pos += d; }

			float	get_offset_angle()	const { return offset_angle; }
			void	set_offset_angle(float a) { offset_angle = a; }
			void	add_offset_angle(float d) { offset_angle += d; }
		};

		class static_rig : public camera_rig {
		private:
			void sync_rig() override {
				float angle = anchor->angle + offset_angle;
				cam->set_up({ sinf(angle), cosf(angle), 0 });
			}

		public:
			static_rig(const object* anchor, float2 offset = float2{ 0 }, float angle = 0) : camera_rig(anchor, offset, angle) {}
		};
	}

	namespace full {
		class camera_rig : public generic::camera_rig {
		private:
			void sync_anchor() override { cam->set_eye(anchor->get_pos() + offset_trans.get_pos()); }
		protected:
			const object* anchor;
			transform offset_trans;

#ifdef _DEBUG 
			void _init_debug() override {
				std::unique_ptr<full::object> debug_mdl = std::make_unique<full::object>();
				debug_mdl->set_model(create_cube());
				debug_mdl->set_scl(float3{ 0.25f });
				debug_mdl->add_settings({ rasterize::WIRE });
				debug_obj = std::move(debug_mdl);
			}

			void _sync_debug() override {
				full::object* debug_mdl = static_cast<full::object*>(debug_obj.get());
				transform t = anchor->get_trans() * offset_trans;
				t.set_scl(offset_trans.get_scl());
				debug_mdl->set_trans(t);
			}
#endif

		public:
			camera_rig(const object* anchor, transform offset = transform()) : anchor(anchor), offset_trans(offset) {}
			virtual ~camera_rig() {}

			const transform& get_transform() { return offset_trans; }
			void set_transform(const transform& t) { offset_trans = t; }
			void add_transform(const transform& d) { offset_trans += d; }

		};

		class static_rig : public camera_rig {
		private:
			void sync_rig() override {
				float3x3 rotation(anchor->get_qtn() * offset_trans.get_qtn());
				cam->set_focus(cam->get_eye() + rotation.forward());
				cam->set_up(rotation.up());
			}

		public:
			static_rig(const object* anchor, transform offset = transform()) : camera_rig(anchor, offset) {}
		};
	}

	class orbit_rig : public full::camera_rig {
	private:
		float distance;
		float min_distance;
		float max_distance;

		void sync_rig() override {
			float3x3 rotation(anchor->get_qtn() * offset_trans.get_qtn());
			float3 camera_pos = rotation.inv_rotate({ 0, 0, -distance });
			offset_trans.set_pos(camera_pos);
			cam->set_focus(anchor->get_pos());
			cam->set_up(float3x3(anchor->get_qtn()).up());
		}

	public:
		orbit_rig(const full::object* anchor, float distance, float min_distance = 0, float max_distance = -1) : 
			full::camera_rig(anchor), distance(distance), min_distance(min_distance), max_distance(max_distance == -1 ? distance : max_distance) {}

		float get_distance() { return distance; }
		void set_distance(float d) { distance = clamp(min_distance, d, max_distance); }
		void add_distance(float d) { set_distance(distance + d); }
	};

}