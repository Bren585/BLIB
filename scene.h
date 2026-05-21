#pragma once

#include "status.h"
#include "canvas.h"
#include "window.h"
#include "manager.h"
#include "lights.h"

#define DEFAULT_BACKGROUND		BLACK

namespace BLIB {

	class camera;

	namespace generic { class scene; }
	namespace lighting { void make_shadows(const generic::scene*, const std::vector<light>*); }

	namespace generic {
		class scene : public status, public canvas {
			friend void lighting::make_shadows(const generic::scene*, const std::vector<light>*);
		private:
			camera*				active_camera	= nullptr;
			std::vector<light>	lights;
			environment_lights	scene_lights;

			virtual void _on_resize() {} // engine use only
			void on_load() override { force_sleep(); }

		protected:
			virtual void draw(render_settings = {}) const	{}
			virtual void on_resize()						{}

		public:
			scene(float2 size = window::size()) : canvas(size) {
				pivot = C_BL;
				set_background(DEFAULT_BACKGROUND);
			}

			void resize(float2 size) { canvas::resize(size); _on_resize(); on_resize(); }

			void						set_post_effects	(render_settings rs)	{ object::set_settings(rs); }
			void						add_post_effects	(render_settings rs)	{ object::add_settings(rs); }
			render_settings				get_post_effects	() const				{ return object::get_settings(); }

			void						set_camera			(camera* cam)			{ active_camera = cam;	}
			camera*						get_camera			()						{ return active_camera; }
			const camera*				get_camera			() const				{ return active_camera; }

			environment_lights&			get_scene_lights	()						{ return scene_lights; }
			const environment_lights&	get_scene_lights	() const				{ return scene_lights; }

			std::vector<light>&			get_lights			()						{ return lights; }
			const std::vector<light>&	get_lights			() const				{ return lights; }

			virtual ~scene() = default;
			virtual void init	()						override = 0;
			virtual void update	(float elapsed_time)	override = 0;
			virtual void idle	(float elapsed_time)	override = 0;
			//// inherited from status
			//virtual void kill() {}
			//virtual void wake() {}

			virtual void _render(const camera* cam) const = 0;
			virtual void render(const camera* cam = nullptr, const environment_lights* scene_lights = nullptr, const std::vector<light>* lights = nullptr) const;
		};
	}

	namespace flat {
		class scene : public generic::scene {
			friend class camera_scene;
		private:
			void _render(const camera* cam) const override;
		public:
			scene(float2 size = window::size()) : generic::scene(size) {}
			virtual ~scene() {}
		};
	}

	namespace full {
		class scene : public generic::scene {
		private:
			static constexpr UINT geometry_layer_count = 5;

			Microsoft::WRL::ComPtr<ID3D11Buffer> point_buffer;
			std::unique_ptr<render_target::view> geometry_buffer[MAX_VIEWS];
			void _on_resize() override { for (int i = 0; i < geometry_layer_count; i++) geometry_buffer[i]->resize(get_view_size()); }

			void _render(const camera* cam) const override;

		protected:
			void opaque_pass		(const camera* cam) const;
			void lighting_pass		()					const;
			void transparent_pass	(const camera* cam) const;

			virtual void draw_transparent() const {}

		public:
			scene(float2 size = window::size()) : generic::scene(size) {
				geometry_buffer[0] = std::make_unique<render_target::view>(size);						// Albedo
				geometry_buffer[1] = std::make_unique<render_target::view>(size);						// Normal
				geometry_buffer[2] = std::make_unique<render_target::view>(size);						// ORM
				geometry_buffer[3] = std::make_unique<render_target::view>(size);						// Emissive
				geometry_buffer[4] = std::make_unique<render_target::view>(size, VIEW_FORMAT_FLOAT);	// World Position
				geometry_buffer[5] = nullptr;
				geometry_buffer[6] = nullptr;
				geometry_buffer[7] = nullptr;
				make_point_buffer(point_buffer.GetAddressOf());
			}
			virtual ~scene() {}

#ifdef _DEBUG
			render_target::view* get_gbuffer_slice(int i) { return geometry_buffer[i].get(); }
#endif
		};
	}

	class camera_scene : protected flat::scene {
	private:
		generic::scene* target_scene;

	public:
		camera_scene(float2 size = window::size(), generic::scene* scene = nullptr) : flat::scene(size), target_scene(scene) {}

		void set_scene(generic::scene* scene) { target_scene = scene; }

		void draw	(render_settings)				const override;
		void render	(const camera* cam = nullptr,	const environment_lights* scene_lights = nullptr, const std::vector<light>*lights = nullptr)	const override;

	};
}