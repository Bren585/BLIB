#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <vector>
#include <map>

#include "materials.h"
#include "render_settings.h"

// IF THESE CONSTANTS ARE CHANGED THEY MUST BE UPDATED IN THE SHADERS
#define MAX_BONE_INF	4

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

	// Mesh

	struct vertex {
		float3		position;
		float3		normal{ 0, 1, 0 };
		float2		texcoord;
		float4		tangent{ 1, 0, 0, 1 };

		float		bone_weights[MAX_BONE_INF]{ 1, 0, 0, 0 };
		uint32_t	bone_indices[MAX_BONE_INF]{};

		SERIALIZE(position, normal, texcoord, bone_weights, bone_indices)
	};

	struct bone_inf {
		uint32_t	index;
		float		weight;
		SERIALIZE(index, weight)
	};

	struct skeleton {
		struct bone {
			uint64_t	unique_id{ 0 };
			string		name;
			int64_t		parent_index{ -1 };
			int64_t		node_index{ 0 };

			float4x4 offset_transform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

			bool is_orphan() const { return parent_index < 0; }

			SERIALIZE(unique_id, name, parent_index, node_index, offset_transform)
		};
		std::vector<bone> bones;

		inline int64_t indexof(uint64_t unique_id) const {
			int64_t index{ 0 };
			for (const bone& bone : bones) {
				if (bone.unique_id == unique_id) { return index; }
				index++;
			}
			return -1;
		}

		SERIALIZE(bones)
	};

	struct mesh {
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer>				vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				index_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				bone_buffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	bone_srv;

		mutable bool unwrapped = false;
		mutable std::vector<triangle> triangle_mesh;

		void unwrap_triangles() const;
	public:
		ID3D11Buffer* const*				get_vertices()	const { return vertex_buffer.GetAddressOf(); }
		ID3D11Buffer*						get_indices()	const { return index_buffer.Get(); }
		ID3D11ShaderResourceView* const*	get_bones()		const { return bone_srv.GetAddressOf(); }

		struct subset {
			uint64_t	material_unique_id{ 0 };
			string		material_name;
			uint32_t	start_index_location{ 0 };
			uint32_t	index_count{ 0 };

			SERIALIZE(material_unique_id, material_name, start_index_location, index_count)
		};
		std::vector<subset>		subsets;
		std::vector<vertex>		vertices;
		std::vector<uint32_t>	indices;

		string		name;
		uint64_t	unique_id{ 0 };
		int64_t		node_index{ 0 };

		skeleton bind_pose;

		float4x4 default_global_transform = MATRIX_ID;

		float3 minimum;
		float3 maximum;

		float3 size() const { return maximum - minimum; }

		void create_buffers();
		void update_bone_buffer(std::vector<float4x4>& bone_transforms, bool dump = false) const;

		const std::vector<triangle>& peek_triangles() const { if (!unwrapped) { unwrap_triangles(); } return triangle_mesh; }

		uint32_t ray_collision(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const;

		SERIALIZE(unwrapped, triangle_mesh, unique_id, name, node_index, vertices, indices, subsets, bind_pose, default_global_transform, minimum, maximum)
	};

	// Animation

	struct animation {
		struct keyframe {
			struct node {
				float4x4 global_transform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
				transform local_transform;

				SERIALIZE(global_transform, local_transform)
			};
			std::vector<node> nodes;
		
			SERIALIZE(nodes)

		};
		std::vector<keyframe> sequence;
		string name;
		float sampling_rate{ 0 }; // A value of 0 will be set to the default sampling rate

		SERIALIZE(name, sampling_rate, sequence)

	private:
		float	timer	{ 0.0f };
		bool	loop	{ false };

	public:
		inline bool update(float elapsed_time) {
			timer += elapsed_time;
			if (get_frame() > sequence.size() - 1) {
				timer -= get_duration();
				return loop;
			}
			return true;
		}
		inline void reset()					{ timer = 0; }
		inline void set_loop(bool l = true)	{ loop = l; }

		inline float			get_duration()	const	{ return (sequence.size() - 1) / sampling_rate;							}
		inline int				get_frame()		const	{ int frame = static_cast<int>(timer * sampling_rate); return frame;	}
		inline const keyframe& get_keyframe()	const {
			int frame = get_frame();
			return sequence.at(frame > sequence.size() - 1 ? (int)sequence.size() - 1 : frame);
		}
	};

	inline void blend_animations(const animation::keyframe& in_key_1, const animation::keyframe& in_key_2, float factor, animation::keyframe& out_key) {
		const size_t node_count{ in_key_1.nodes.size() };
		out_key.nodes.clear();
		out_key.nodes.resize(node_count);
		for (size_t node_index = 0; node_index < node_count; node_index++) {
			out_key.nodes[node_index].global_transform = lerp(in_key_1.nodes[node_index].global_transform, in_key_2.nodes[node_index].global_transform, factor);
		}
	}

	// Model

	class model {
	public:
		struct constants {
			float4x4	world;
			color		material_color;
			//float4x4	bone_transforms[MAX_BONES]{MATRIX_ID};
		};

	private:
		float	transition_timer	= 0;
		float	transition_duration	= 0;
		int		sequence			= -2;
		int		transition			= -2;

		static inline animation::keyframe temp_frame;
		
	protected:

		static inline std::wstring filepath = L"";

		Microsoft::WRL::ComPtr<ID3D11Buffer>	constant_buffer;

		std::vector<mesh>								meshes;
		std::unordered_map<uint64_t, material>			materials;
		std::vector<animation>							animations;
		coordinate_system								coord_sys;
		string											vs_cso;
		std::vector<D3D11_INPUT_ELEMENT_DESC>			input_element_desc;

		void create_shaders(std::string cso);

		inline virtual model* clone_impl() const = 0;

	public:
		auto clone() const { return std::unique_ptr<model>(clone_impl()); }
		virtual ~model() = default;

		static inline void			set_filepath(std::wstring path) { filepath = path; }
		static inline std::wstring	get_filepath()					{ return filepath; }

		inline virtual void render(const float4x4& world, const color& material_color) const = 0;
		
		// Animation

		void update(float elapsed_time);

		void animate(int animation_id, float enter_time = 0.0f, bool loop = false);

		void stop_animation() { sequence = transition = -2; }

		bool is_animating() { return sequence != -2; }

		const animation::keyframe* get_keyframe() const;

		// Textures

		void force_reload_textures() { for (auto& material : materials) { material.second.force_construct(); } }

		// Physics / Collision

		inline virtual float3 get_size() const = 0;

		virtual const std::vector<triangle>& peek_triangles() const = 0;

		virtual uint32_t ray_collision(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const = 0;
	
		SERIALIZE(meshes, materials, animations, coord_sys, vs_cso)
	};

	// Primitives

	void set_primitive_detail(int level = 12);
	model* create_cube(float3 min = -0.5f, float3 max = 0.5f);
	model* create_sphere();
	model* create_cylinder();
	model* create_capsule(float height = 0.5f, float radius = 0.5f);
	model* create_quad();

	void load_texture(model* dest, const string filename, texture_type slot = texture_map, float3* out_aspect = nullptr);

	class sprite;
	void copy_texture(model* dest, const sprite* spr, float3* out_aspect = nullptr);

	// FBX

	model* load_fbx(const char* fbx_filename,		bool triangulate, coordinate_system sys);
	model* load_fbx(const char* cereal_filename,	bool full_filepath = false);
	void add_animations_fbx(model* target, const char* animation_filename, float sampling_rate = 0);

}