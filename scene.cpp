#include "pch.h"
#include "scene.h"
#include "lighting.h"
#include "camera.h"

namespace BLIB {
	namespace generic {
		void scene::render(const camera* cam, const environment_lights* scene_lights, const std::vector<light>* lights) const {
			const camera* active = nullptr;
			if (cam) active = cam;
			else if (active_camera) active = active_camera;
			if (lights) { lighting::bind_lights(this, scene_lights, lights); }
			_render(active);
		}
	}

	namespace flat {
		void scene::_render(const camera* cam) const {
			annotate("flat::scene");
			canvas::clear();
			canvas::focus();
			draw();
			canvas::unfocus();
			canvas::render_to_main();
		}
	}

	namespace full {

		void scene::opaque_pass(const camera* cam) const {
			annotate("opaque pass");
			for (auto& layer : geometry_buffer) { if (layer) layer->clear(COLORLESS); }
			render_target::quick_focus(geometry_buffer);
			cam->bind();
			draw({pixel_shader("deferred_begin")});
		}

		void scene::lighting_pass() const {
			lighting::bind_lights(this);

			annotate("lighting pass");
			for (int i = 0; i < geometry_layer_count; i++) { if (geometry_buffer[i]) geometry_buffer[i]->bind_SRV(i); }
			//geometry_buffer[0]->bind_depth(geometry_layer_count);

			(render_settings{ pixel_shader("deferred_lighting"), stencil::DEPTH_NONE } & sprite::default_rs()).set();

			update_point_buffer(point_buffer.Get());

			draw_points(point_buffer.GetAddressOf());
		}

		void scene::transparent_pass(const camera* cam) const {
			annotate("transparent pass");
			cam->bind();
			draw_transparent();
		}

		void scene::_render(const camera* cam) const {
			if (!get_camera()) return;
			annotate("full::scene");
			canvas::clear();
			canvas::focus(FOCUS_DEPTH);
			opaque_pass(cam);
			canvas::focus();
			lighting_pass();
			transparent_pass(cam);
			canvas::unfocus();
			canvas::render_to_main();
		}
	}

	void camera_scene::draw(render_settings rs) const {
		render_settings old_effects = target_scene->get_post_effects();
		target_scene->set_post_effects({ vertex_shader{DEFAULT_FLAT} });
		target_scene->render(get_camera(), &get_scene_lights(), &get_lights());
		target_scene->set_post_effects(old_effects);
	}

	void camera_scene::render(const camera* cam, const environment_lights* scene_lights, const std::vector<light>* lights) const {
		static_cast<const generic::scene*>(this)->_render(get_camera() ? get_camera() : cam);
	}
}