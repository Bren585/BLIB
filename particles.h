#pragma once
#include "model.h"
#include "texture_animation.h"

#define DEFAULT_MAX_PARTICLES	128
#define MAX_MAX_PARTICLES		512

namespace BLIB {

	class particles : public model, public mesh {
	private:
		void com_setup();

		virtual particles* clone_impl() const = 0;

		void render(const float4x4& world, const color& material_color) const override;
		void _update(float elapsed_time) override;
	
		bool active = true;

		inline static const std::vector<triangle> dummy_triangles = std::vector<triangle>();

	protected:
		struct point {
			float4 position;
			float2 size		{ 1, 1 };
			float2 uv_size	{ 1, 1 };
			float2 uv_index;
		};

		struct particle {
			point p;
			float3 velocity;
			float lifetime			= 0;
			float max_lifetime;
			float2 scale			{ 1, 1 };
			texture_animator animator;
		};

		std::vector<particle>	particle_buffer;
		std::vector<point>		point_buffer;
		uint max_particles = DEFAULT_MAX_PARTICLES;

		virtual void update_each(float elapsed_time, particle& particle) = 0;
		virtual void emit		(float elapsed_time) = 0;

	public:
		particles(const particles& o) : particle_buffer(o.particle_buffer), point_buffer(o.point_buffer), active(o.active), max_particles(o.max_particles) { com_setup(); }
		particles() { com_setup(); }
		virtual ~particles() {}

		void start	() { active = true; }
		void stop	() { active = false; }

		bool is_active		() { return active; }
		bool has_particles	() { return particle_buffer.size() > 0; }

		inline void load_texture(const string& filename, texture_type slot = texture_map)										{ materials[0].textures[slot] = std::make_unique<material_texture_file>(filename);	materials[0].textures[slot]->construct(); }
		inline void make_texture(color c, texture_type slot = texture_map)														{ materials[0].textures[slot] = std::make_unique<material_texture_dummy>(c);		materials[0].textures[slot]->construct(); }
		inline void copy_texture(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& SRV, texture_type slot = texture_map)	{ SRV.CopyTo(materials[0].textures[slot]->data->get_release_SRV());					materials[0].textures[slot]->construct(); }

		inline auto& get_texture(texture_type slot = texture_map) { return materials[0].textures[slot]; }

		void set_max_particles(uint count) { max_particles = count < MAX_MAX_PARTICLES ? count : MAX_MAX_PARTICLES; }

		inline float3						get_size		() const override { return float3{0}; }
		inline const std::vector<triangle>& peek_triangles	() const override { return dummy_triangles; }
		inline uint32_t						ray_collision	(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const override { return 0; }

		render_settings default_rs() const override { return { vertex_shader("billboard"), geometry_shader("billboard") }; }

	};

}