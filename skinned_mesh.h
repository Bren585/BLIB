#pragma once

#include <fbxsdk.h>
#include <vector>

#include "model.h"
#include "string.h"

namespace BLIB {

	class skinned_mesh : public model {
	public:
		struct scene_view {
			struct node {
				uint64_t				unique_id{ 0 };
				string					name;
				FbxNodeAttribute::EType attribute{ FbxNodeAttribute::EType::eUnknown };
				int64_t					parent_index{ -1 };

				SERIALIZE(unique_id, name, attribute, parent_index)
			};
			std::vector<node> nodes;

			int64_t index_of(uint64_t unique_id) const {
				int64_t index{ 0 };
				for (const node& node : nodes) { if (node.unique_id == unique_id) { return index; } index++; }
				return -1;
			}

			SERIALIZE(nodes)
		};

	private:
		friend model* load_fbx(const char* fbx_filename, bool trianglate, coordinate_system sys);
		friend model* load_fbx(const char* cereal_filename, bool full_filepath);
		friend skinned_mesh* cereal::access::construct();

		void fetch_meshes		(FbxScene* fbx_scene);
		void fetch_materials	(string fbx_filename,		FbxScene* fbx_scene);
		void fetch_bones		(const FbxMesh* fbx_mesh,	std::vector<std::vector<bone_inf>>& bones);
		void fetch_skeleton		(const FbxMesh* fbx_mesh,	skeleton& bind_pose);
		void fetch_attachment	(FbxNode* fbx_node,			skeleton& bind_pose);
		void fallback_skeleton	(skeleton& bind_pose);
		void fetch_animations	(FbxScene* fbx_scene,		std::unordered_map<string, animation>& animation_clips, float sampling_rate);

		void create_com_objects();

		mutable bool cached_triangles = false;
		mutable std::vector<triangle> triangle_cache;

		void cache_triangles() const {
			if (cached_triangles) { return; }
			cached_triangles = true;
			triangle_cache.clear();

			for (const auto& m : meshes) {
				for (const auto& t : m.peek_triangles()) {
					triangle_cache.push_back(t);
				}
			}
		}

		skinned_mesh() {}
	protected:
		string fbx_name;
		scene_view scene_view;
		float3 minimum;
		float3 maximum;

		virtual skinned_mesh* clone_impl() const override { return new skinned_mesh(*this); }
		void render(const float4x4& world, const color& material_color) const override;

	public:
		skinned_mesh(const char* fbx_filename, bool triangulate = false, coordinate_system sys = RH_Y);
		virtual ~skinned_mesh() = default;

		render_settings default_rs() const override { return vertex_shader("skinned_full"); };

		bool append_animations(string animation_filename, float sampling_rate);
		void update_animation(animation::keyframe& keyframe);

		inline float3 get_size() const override { return maximum - minimum; }

		inline const std::vector<triangle>& peek_triangles() const override { cache_triangles(); return triangle_cache; }

		uint32_t ray_collision(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const override;

		SERIALIZE_BASE(model, fbx_name, scene_view, minimum, maximum)
	};

}

CONSTRUCT_BASE(BLIB::skinned_mesh, BLIB::model, self.scene_view, self.minimum, self.maximum)

