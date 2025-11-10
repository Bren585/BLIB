#include "pch.h"
#include "scene.h"
#include "lighting.h"
#include "camera.h"

namespace BLIB {
	namespace generic {
		void scene::render(const camera* cam, const environment_lights* scene_lights, const std::vector<light>* lights) const {
			if (cam) cam->bind();
			else if (active_camera) active_camera->bind();
			if (lights) { lighting::bind_lights(this, scene_lights, lights); }
			_render();
		}
	}

	namespace flat {
		void scene::_render() const {
			canvas::clear();
			canvas::focus();
			draw({});
			canvas::unfocus();
			canvas::render_to_main();
		}
	}

	namespace full {

		void scene::opaque_pass() const {
			annotate("opaque pass");
			for (auto& layer : geometry_buffer) { if (layer) layer->clear(COLORLESS); }
			render_target::quick_focus(geometry_buffer);
			draw({pixel_shader("deferred_begin")});
		}

		void scene::lighting_pass() const {
			lighting::bind_lights(this);

			annotate("lighting pass");
			for (int i = 0; i < geometry_layer_count; i++) { if (geometry_buffer[i]) geometry_buffer[i]->bind_SRV(i); }
			//geometry_buffer[0]->bind_depth(geometry_layer_count);

			render_settings{ pixel_shader("deferred_lighting"), vertex_shader(DEFAULT_FLAT) }.set();

			uint32_t stride{ sizeof(sprite::vertex) };
			uint32_t offset{ 0 };
			device::context()->IASetVertexBuffers(0, 1, quad_buffer.GetAddressOf(), &stride, &offset);
			device::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			device::context()->Draw(4, 0);
		}

		void scene::transparent_pass() const {
			annotate("transparent pass");
			draw_transparent();
		}

		void scene::_render() const {
			if (!get_camera()) return;
			annotate("full::scene");
			opaque_pass();
			canvas::clear();
			canvas::focus();
			lighting_pass();
			transparent_pass();
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
		static_cast<const generic::scene*>(this)->_render();
	}
}