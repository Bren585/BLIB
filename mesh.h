#pragma once
#include <vector>
#include "materials.h"
#include "skinning_mode.h"

#define MAX_BONE_INF 4

namespace BLIB {

	struct bone_inf {
		uint32_t	index;
		float		weight;
		SERIALIZE(index, weight)
	};

	struct skeleton {
		struct bone {
			uint64_t	unique_id		{ 0 };
			string		name;
			int64_t		parent_index	{ -1 };
			int64_t		node_index		{ 0 };

			float4x4 offset_transform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

			bool is_orphan() const { return parent_index < 0; }

			SERIALIZE(unique_id, name, parent_index, node_index, offset_transform)
		};
		std::vector<bone> bones;

		inline int64_t index_of(uint64_t unique_id) const {
			int64_t index{ 0 };
			for (const bone& bone : bones) {
				if (bone.unique_id == unique_id) { return index; }
				index++;
			}
			return -1;
		}

		inline int64_t index_of(string name) const {
			int64_t index{ 0 };
			for (const bone& bone : bones) {
				if (bone.name == name) { return index; }
				index++;
			}
			return -1;
		}

		SERIALIZE(bones)
	};

	struct mesh {
		struct vertex {
			float3		position;
			float3		normal{ 0, 1, 0 };
			float2		texcoord;
			float4		tangent{ 1, 0, 0, 1 };

			float		bone_weights[MAX_BONE_INF]{ 1, 0, 0, 0 };
			uint32_t	bone_indices[MAX_BONE_INF]{};

			SERIALIZE(position, normal, texcoord, bone_weights, bone_indices)
		};
	protected:
		mutable Microsoft::WRL::ComPtr<ID3D11Buffer>		vertex_buffer;
		mutable Microsoft::WRL::ComPtr<ID3D11Buffer>		index_buffer;
#ifdef SKIN_GPU
		Microsoft::WRL::ComPtr<ID3D11Buffer>				bone_buffer;
#endif

	private:
		mutable bool unwrapped = false;
		mutable std::vector<triangle> triangle_mesh;
		void unwrap_triangles() const;

		mutable bool bone_transform_vector_prepped = false;
		mutable std::vector<matrix> scratch_bone_transforms;

		mutable bool bone_normal_vector_prepped = false;
		mutable std::vector<matrix> scratch_bone_normals;

		mutable bool vertex_vector_prepped = false;
		mutable std::vector<vertex> scratch_vertices;

	public:
		ID3D11Buffer* const* get_vertices()	const { return vertex_buffer.GetAddressOf(); }
		ID3D11Buffer* get_indices()	const { return index_buffer.Get(); }
		ID3D11Buffer* const* get_bone_buffer() const { return bone_buffer.GetAddressOf(); }

		struct subset {
			uint64_t	material_unique_id		{ 0 };
			string		material_name;
			uint32_t	start_index_location	{ 0 };
			uint32_t	index_count				{ 0 };

			SERIALIZE(material_unique_id, material_name, start_index_location, index_count)
		};
		std::vector<subset>		subsets;
		std::vector<vertex>		vertices;
		std::vector<uint32_t>	indices;

		string		name;
		uint64_t	unique_id{ 0 };
		int64_t		node_index{ 0 };

		skeleton bind_pose;

		float4x4 default_global_transform			= MATRIX_ID;
		float4x4 inverse_default_global_transform	= MATRIX_ID;

		float3 minimum;
		float3 maximum;

		float3 size() const { return maximum - minimum; }

		void create_buffers();
		void update_buffers(const std::vector<vertex>& vertices) const;
		void update_bone_buffer(std::vector<float4x4>& bone_transforms) const;

		const std::vector<triangle>& peek_triangles		() const { if (!unwrapped)						{ unwrap_triangles();																				} return triangle_mesh;				}
		std::vector<matrix>& get_scratch_bone_transforms() const { if (!bone_transform_vector_prepped)	{ scratch_bone_transforms.resize(bind_pose.bones.size());	bone_transform_vector_prepped	= true; } return scratch_bone_transforms;	}
		std::vector<matrix>& get_scratch_bone_normals	() const { if (!bone_normal_vector_prepped)		{ scratch_bone_normals.resize(bind_pose.bones.size());		bone_normal_vector_prepped		= true;	} return scratch_bone_normals;		}
		std::vector<vertex>& get_scratch_vertices		() const { if (!vertex_vector_prepped)			{ scratch_vertices.resize(vertices.size());					vertex_vector_prepped			= true;	} return scratch_vertices;			}

		uint32_t ray_collision(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const;

		SERIALIZE(unwrapped, triangle_mesh, unique_id, name, node_index, vertices, indices, subsets, bind_pose, default_global_transform, minimum, maximum)
	};

}