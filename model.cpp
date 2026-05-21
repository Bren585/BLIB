#include "pch.h"
#include "model.h"
#include "shader.h"
#include "skinned_mesh.h"
#include "geometric_primitive.h"
#include "billboard.h"
#include "sprite.h"

#include <algorithm>
#include <execution>

namespace BLIB {

	// Model 
	 
	void model::create_shaders(string vs_cso) {
		HRESULT hr{ S_OK };

		_ASSERT_EXPR_A(input_element_desc.size() > 0, "Shader Inputs Not Set");

		shader::load_full(vs_cso, input_element_desc.data(), static_cast<UINT>(input_element_desc.size()));

		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth = sizeof(constants);
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;

		hr = device::get()->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf()); VERIFY;
	}

	// Model Animation

	void model::update(float elapsed_time) {
		_update(elapsed_time);
		update_animation(elapsed_time);
	}

	void model::update_animation(float elapsed_time) {
		bool was_animating = is_animating();
		bool skip = false;
		if (transition != NO_ANIMATION) {
			if (!animations.at(transition).update(elapsed_time)) { transition = NO_ANIMATION; }
			else {
				transition_timer += elapsed_time;
				if (transition_timer > transition_duration) { sequence = transition; transition = NO_ANIMATION; skip = true; }
			}
		}
		if (sequence != NO_ANIMATION && !skip) {
			if (!animations.at(sequence).update(elapsed_time) && transition != NO_ANIMATION) { sequence = transition; transition = NO_ANIMATION; }
		}
		
		update_meshes(was_animating);
	}

	std::vector<matrix> bone_transforms_id = { DirectX::XMLoadFloat4x4(&matrix_id) };

	void skin_mesh(const mesh& m, const animation::keyframe& frame, const matrix& coord_system, const bool animating) {
		const float4x4& mesh_global = (animating ? frame.at((int)m.node_index).global_transform : m.default_global_transform);
		const matrix	world = DirectX::XMLoadFloat4x4(&mesh_global) * coord_system;
		const matrix	world_normal = transpose(inverse3x3(world));

		matrix* bone_transforms = nullptr;
		matrix* bone_normals = nullptr;
		if (animating) {
			bone_transforms = m.get_scratch_bone_transforms().data();
			bone_normals = m.get_scratch_bone_normals().data();

			const size_t bone_count = m.bind_pose.bones.size();
			for (size_t i = 0; i < bone_count; i++) {
				bone_transforms[i] =
					DirectX::XMLoadFloat4x4(&m.bind_pose.bones[i].offset_transform) *
					DirectX::XMLoadFloat4x4(&frame.at((int)m.bind_pose.bones[i].node_index).global_transform) *
					DirectX::XMLoadFloat4x4(&m.inverse_default_global_transform);

				bone_normals[i] = transpose(inverse3x3(bone_transforms[i]));
			}
		}
		else {
			bone_transforms = bone_normals = bone_transforms_id.data();
		}

		std::vector<mesh::vertex>& vertices = m.get_scratch_vertices();
		const size_t vertex_count = m.vertices.size();
		for (size_t i = 0; i < vertex_count; i++) {
			const mesh::vertex& v = m.vertices[i];
			float3 weighted_position;
			float3 weighted_normal;
			float4 weighted_tangent;
			float total_weight = 0.0f;
			for (int w = 0; w < MAX_BONE_INF; w++) {
				const float	weight = v.bone_weights[w];
				if (is_zero(weight)) continue;
				total_weight += weight;
				const uint bone_index = animating ? v.bone_indices[w] : 0;
				const matrix& bone_transform = bone_transforms[bone_index];
				weighted_position += weight * mul(v.position, bone_transform);
				const matrix& bone_normal_transform = bone_normals[bone_index];
				weighted_normal += weight * mul_normal(v.normal, bone_normal_transform);
				weighted_tangent += weight * mul_normal(v.tangent, bone_normal_transform);
			}
			if (is_zero(total_weight)) {
				weighted_position = v.position;
				weighted_normal = v.normal;
				weighted_tangent = v.tangent;
			}
			mesh::vertex& out = vertices[i];
			out.position = mul(weighted_position, world);
			out.normal = mul_normal(weighted_normal, world_normal);
			out.tangent = mul_normal(weighted_tangent, world_normal);
			out.texcoord = v.texcoord;
		}
	}

	void model::update_meshes(bool force) {
		bool animating = is_animating();
		if (animating || force) {
			animation::keyframe blended_frame;
			if (animating) {
				if (transition != NO_ANIMATION) { blend_keyframes(animations[sequence].get_keyframe(), animations[transition].get_keyframe(), transition_timer / transition_duration, blended_frame); }
				else { blended_frame = animations[sequence].get_keyframe(); }
			}

#ifdef SKIN_CPU
			const matrix coord_system = DirectX::XMLoadFloat4x4(&coordinate_system_transforms[coord_sys]);

			std::for_each(std::execution::par, meshes.begin(), meshes.end(), [&](const mesh& m) { skin_mesh(m, blended_frame, coord_system, animating); });

			for (const mesh& mesh : meshes) {
				mesh.update_buffers(mesh.get_scratch_vertices());
			}
#else
			for (const mesh& mesh : meshes) {
				const size_t bone_count = mesh.bind_pose.bones.size();
				std::vector<float4x4> bone_transforms;
				bone_transforms.resize(bone_count);
				for (size_t i = 0; i < bone_count; i++) {
					if (animating)
						DirectX::XMStoreFloat4x4(&bone_transforms[i],
							DirectX::XMLoadFloat4x4(&mesh.bind_pose.bones[i].offset_transform) *
							DirectX::XMLoadFloat4x4(&blended_frame.at((int)mesh.bind_pose.bones[i].node_index).global_transform) *
							DirectX::XMLoadFloat4x4(&mesh.inverse_default_global_transform)
						);
					else bone_transforms[i] = matrix_id;
				}
				mesh.update_bone_buffer(bone_transforms);
			}
#endif
		}
	}

	void model::animate(string animation_name, float enter_time, bool loop, float playback) {
		animations.at(animation_name).set_loop(loop);
		animations.at(animation_name).set_playback(playback);
		animations.at(animation_name).reset();
		transition = animation_name;
		transition_duration = enter_time;
		transition_timer = 0.0f;
	}

	const animation::keyframe* model::get_keyframe() const {
		if (sequence == NO_ANIMATION) { return nullptr; }
		if (transition != NO_ANIMATION) {
			blend_keyframes(animations.at(sequence).get_keyframe(), animations.at(transition).get_keyframe(), transition_timer / transition_duration, temp_frame);
			return &temp_frame;
		}
		else { return &animations.at(sequence).get_keyframe(); }
	}

	// Primitives

	int primitive_detail = 12;
	void set_primitive_detail(int level) { primitive_detail = level; }

	model* create_cube			(float3 min, float3 max)		{ return new custom_cube	(min, max);							}
	model* create_sphere		()								{ return new sphere			(primitive_detail);					}
	model* create_cylinder		()								{ return new cylinder		(primitive_detail);					}
	model* create_capsule		(float height, float radius)	{ return new capsule		(height, radius, primitive_detail); }
	model* create_quad			()								{ return new quad;												}
	model* create_rect_pyramid	()								{ return new rect_pyramid;										}	
	model* create_billboard		(float2 size)					{ return new billboard(size);									}

	void load_texture(model* dest, const string filename, texture_type slot, float3* out_aspect) {
		geometric_primitive* primitive = dynamic_cast<geometric_primitive*>(dest);
		if (primitive) {
			primitive->load_texture(filename, slot);
			float2 size = primitive->get_texture(slot)->data->get_size();
			float aspect = size.x / size.y;
			if (out_aspect) *out_aspect = { aspect, 1, 1 };
		}
	}

	void copy_texture(model* dest, const sprite* spr, float3* out_aspect) {
		geometric_primitive* primitive = dynamic_cast<geometric_primitive*>(dest);
		if (primitive) {
			primitive->copy_texture(spr->peek_SRV());
			float2 size = spr->get_size();
			float aspect = size.x / size.y;
			if (out_aspect) *out_aspect = { aspect, 1, 1 };
		}
	}

	// FBX

	model* load_fbx(const char* fbx_filename, bool triangulate, coordinate_system sys) {
		// Add filepath and extension
		string filename{ model::get_filepath() };
		filename += fbx_filename;
		filename += ".fbx";

		string cereal_filename(filename.replace_ext("cereal"));
		if (cereal_filename.file_exists()) {
			return load_fbx(cereal_filename, true);
		}
		else {
			std::unique_ptr<skinned_mesh> out = std::make_unique<skinned_mesh>(filename, triangulate, sys);
			CEREAL(cereal_filename, out);
			return out.release();
		}
	}

	model* load_fbx(const char* cereal_filename, bool full_filepath) {
		string filename = cereal_filename;
		if (!full_filepath) {
			// Add filepath and extension
			filename = model::get_filepath();
			filename += cereal_filename;
			filename += ".cereal";
		}
		_ASSERT_EXPR(filename.file_exists(), L"No Cereal File Exists");
		std::unique_ptr<skinned_mesh> out;
		{
			UNCEREAL(filename, out);
		}
		out->create_com_objects();
		//out->animator.set_hierarchy(out->scene_view.hierarchy);
		return out.release();
	}

	void add_animations_fbx(model* target, const char* animation_filename, float sampling_rate) {
		skinned_mesh* fbx_model = dynamic_cast<skinned_mesh*>(target);
		if (!fbx_model) return;
		string filename{ model::get_filepath() };
		filename += animation_filename;
		filename += ".fbx";
		fbx_model->append_animations(filename, sampling_rate);
	}
}

CEREAL_REGISTER_TYPE(BLIB::skinned_mesh)
CEREAL_REGISTER_POLYMORPHIC_RELATION(BLIB::model, BLIB::skinned_mesh)