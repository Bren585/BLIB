#pragma once
#include <map>
#include <memory>
#include "mesh.h"
#include "model_animation.h"
#include "render_settings.h"

#define NO_ANIMATION "-2"

namespace BLIB {

	// Definitions

	const float4x4 coordinate_system_transforms[]{
		{-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 0:RHS Y-UP
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, // 1:LHS Y-UP
		{-1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 2:RHS Z-UP
		{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }, // 3:LHS Z-UP
	};

	enum coordinate_system {
		RH_Y,
		LH_Y,
		RH_Z,
		LH_Z
	};

	// Model

	class model {
	public:
		struct constants {
			float4x4	world;
			color		material_color;
		};

	private:
		float	transition_timer	= 0;
		float	transition_duration	= 0;
		string	sequence			= NO_ANIMATION;
		string	transition			= NO_ANIMATION;

		static inline animation::keyframe temp_frame;
		
	protected:

		static inline string filepath = L"";

		Microsoft::WRL::ComPtr<ID3D11Buffer>			constant_buffer;

		std::vector<mesh>								meshes;
		std::unordered_map<uint64_t, material>			materials;
		std::unordered_map<string, animation>			animations;
		coordinate_system								coord_sys;
		std::vector<D3D11_INPUT_ELEMENT_DESC>			input_element_desc;

		void create_shaders(string vs_cso);

		inline virtual model* clone_impl() const = 0;

	public:
		auto clone() const { return std::unique_ptr<model>(clone_impl()); }
		virtual ~model() = default;

		static inline void		set_filepath(string path)	{ filepath = path; }
		static inline string	get_filepath()				{ return filepath; }

		virtual render_settings default_rs() const { return vertex_shader("default_full"); };

		virtual void render(const float4x4& world, const color& material_color) const = 0;
		
		// Animation

		void update(float elapsed_time);

		virtual void _update(float elapsed_time) {}

		void update_animation(float elapsed_time);

		void update_meshes(bool force = false);

		void animate(string animation_name, float enter_time = 0.0f, bool loop = false, float playback = 1);

		void stop_animation() { sequence = transition = NO_ANIMATION; }

		bool is_animating() const { return sequence != NO_ANIMATION; }

		float remaining_animation_time() const { if (is_animating()) { return animations.at(sequence).get_remaining_time(); } else { return 0; } }

		const auto& get_animations() const { return animations; }

		const string& get_current_animation() const { return sequence; }

		const animation& get_animation(string name) const { return animations.at(name); }

		const animation::keyframe* get_keyframe() const;

		// Textures

		auto& get_textures() { return materials; }

		void force_reload_textures() { for (auto& material : materials) { material.second.force_construct(); } }

		// Physics / Collision

		inline virtual float3 get_size() const = 0;

		virtual const std::vector<triangle>& peek_triangles() const = 0;

		virtual uint32_t ray_collision(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const = 0;
	
		SERIALIZE(meshes, materials, animations, coord_sys)
	};

	// Primitives

	void set_primitive_detail(int level = 12);
	model* create_cube(float3 min = float3{ -0.5f }, float3 max = float3{ 0.5f });
	model* create_sphere();
	model* create_cylinder();
	model* create_capsule(float height = 0.5f, float radius = 0.5f);
	model* create_quad();
	model* create_rect_pyramid();
	model* create_billboard(float2 size = float2{ 1 });

	void load_texture(model* dest, const string filename, texture_type slot = texture_map, float3* out_aspect = nullptr);

	class sprite;
	void copy_texture(model* dest, const sprite* spr, float3* out_aspect = nullptr);

	// FBX

	model* load_fbx(const char* fbx_filename,		bool triangulate, coordinate_system sys);
	model* load_fbx(const char* cereal_filename,	bool full_filepath = false);
	void add_animations_fbx(model* target, const char* animation_filename, float sampling_rate = 0);

}