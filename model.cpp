#include "pch.h"
#include "model.h"
#include "shader.h"
#include "skinned_mesh.h"
#include "geometric_primitive.h"
#include "sprite.h"

namespace BLIB {

	// Model 

	void model::create_shaders(std::string vs_cso) {
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
		bool was_animating = is_animating();
		bool skip = false;
		if (transition != -2) {
			if (!animations.at(transition).update(elapsed_time)) { transition = -2; } // transition ended before sequence, abort
			else {
				transition_timer += elapsed_time;
				if (transition_timer > transition_duration) { sequence = transition; transition = -2; skip = true; }
			}
		}
		if (sequence != -2 && !skip) {
			if (!animations.at(sequence).update(elapsed_time)) { sequence = transition; transition = -2; }
		}
		
		update_meshes(was_animating);
	}

	void model::update_meshes(bool force) {
		if (is_animating() || force) {
			animation::keyframe blended_frame;
			if (is_animating()) {
				if (transition != -2) { blend_keyframes(animations.at(sequence).get_keyframe(), animations.at(transition).get_keyframe(), transition_timer / transition_duration, blended_frame); }
				else { blended_frame = animations.at(sequence).get_keyframe(); }
			}

			for (const mesh& mesh : meshes) {
				const float4x4& mesh_global		= (is_animating() ? blended_frame.at((int)mesh.node_index).global_transform : mesh.default_global_transform);
				const matrix	world			= DirectX::XMLoadFloat4x4(&mesh_global) * DirectX::XMLoadFloat4x4(&coordinate_system_transforms[coord_sys]);
				const matrix	world_normal	= transpose(inverse3x3(world));

				std::vector<matrix> bone_transforms;
				if (is_animating()) {
					const size_t bone_count = mesh.bind_pose.bones.size();
					bone_transforms.resize(bone_count);
					for (size_t i = 0; i < bone_count; i++) {
						bone_transforms[i] = DirectX::XMMatrixTranspose( // Shader wants transposed
							DirectX::XMLoadFloat4x4(&mesh.bind_pose.bones[i].offset_transform) *
							DirectX::XMLoadFloat4x4(&blended_frame.at((int)mesh.bind_pose.bones[i].node_index).global_transform) *
							DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&mesh.default_global_transform))
						);
					}
				}
				else {
					bone_transforms.push_back(DirectX::XMLoadFloat4x4(&matrix_id));
				}

				std::vector<vertex> vertices;
				const size_t vertex_count = mesh.vertices.size();
				vertices.resize(vertex_count);
				for (size_t i = 0; i < vertex_count; i++) {
					const vertex& v = mesh.vertices.at(i);
					float3 weighted_position;
					float3 weighted_normal;
					float4 weighted_tangent;
					for (int w = 0; w < MAX_BONE_INF; w++) {
						const float	weight			= v.bone_weights[w];
						matrix		bone_transform	= bone_transforms.at(is_animating() ? v.bone_indices[w] : 0);
						weighted_position	+= weight * mul(v.position, bone_transform);
						bone_transform.r[3]  = DirectX::XMVectorSet( 0, 0, 0, 1 );
						weighted_normal		+= weight * mul_normal(v.normal,	bone_transform);
						weighted_tangent	+= weight * mul_normal(v.tangent,	bone_transform);
					}
					vertex& out = vertices.at(i);
					out.position	= mul(weighted_position, world);
					out.normal		= mul_normal(weighted_normal, world_normal);
					out.tangent		= mul_normal(weighted_tangent, world_normal);
					out.texcoord	= v.texcoord;
				}

				mesh.update_buffers(vertices);
			}
		}
	}

	void model::animate(int animation_id, float enter_time, bool loop) {
		animations.at(animation_id).set_loop(loop);
		animations.at(animation_id).reset();
		transition = animation_id;
		transition_duration = enter_time;
		transition_timer = 0.0f;
	}

	const animation::keyframe* model::get_keyframe() const {
		if (sequence == -2) { return nullptr; }
		if (transition != -2) {
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