#include "pch.h"
#include "model.h"
#include "shader.h"
#include "skinned_mesh.h"
#include "geometric_primitive.h"
#include "sprite.h"

namespace BLIB {

	// Mesh

	void mesh::create_buffers() {
		HRESULT hr{ S_OK };

		D3D11_BUFFER_DESC buffer_desc{};
		D3D11_SUBRESOURCE_DATA subresource_data{};

		buffer_desc.ByteWidth = static_cast<UINT>(sizeof(vertex) * vertices.size());
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;

		subresource_data.pSysMem = vertices.data();
		subresource_data.SysMemPitch = 0;
		subresource_data.SysMemSlicePitch = 0;

		hr = device::get()->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.ReleaseAndGetAddressOf()); VERIFY;

		buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		subresource_data.pSysMem = indices.data();

		hr = device::get()->CreateBuffer(&buffer_desc, &subresource_data, index_buffer.ReleaseAndGetAddressOf()); VERIFY;

		if (bind_pose.bones.size() > 0) {
			buffer_desc.ByteWidth = static_cast<UINT>(bind_pose.bones.size() * sizeof(float4x4));
			buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			buffer_desc.CPUAccessFlags = 0;
			buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			buffer_desc.StructureByteStride = sizeof(float4x4);

			hr = device::get()->CreateBuffer(&buffer_desc, nullptr, bone_buffer.GetAddressOf()); VERIFY;

			D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
			srv_desc.Format = DXGI_FORMAT_UNKNOWN;
			srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srv_desc.Buffer.FirstElement = 0;
			srv_desc.Buffer.NumElements = static_cast<UINT>(bind_pose.bones.size());

			hr = device::get()->CreateShaderResourceView(bone_buffer.Get(), &srv_desc, bone_srv.GetAddressOf()); VERIFY;
		}
	}

	void mesh::update_bone_buffer(std::vector<float4x4>& bone_transforms, bool dump) const {
		assert(bone_transforms.size() == bind_pose.bones.size());
		device::context()->UpdateSubresource(bone_buffer.Get(), 0, 0, bone_transforms.data(), 0, 0);
	}

	void mesh::unwrap_triangles() const {
		HRESULT hr{ S_OK };
		
		D3D11_BUFFER_DESC vertex_desc;
		D3D11_BUFFER_DESC index_desc{};

		vertex_buffer->GetDesc(&vertex_desc);
		index_buffer->GetDesc(&index_desc);
		
		vertex_desc.Usage			= index_desc.Usage			= D3D11_USAGE_STAGING;
		vertex_desc.BindFlags		= index_desc.BindFlags		= 0;
		vertex_desc.CPUAccessFlags	= index_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		ID3D11Buffer* vertex_staging_buffer;
		ID3D11Buffer* index_staging_buffer;

		hr = device::get()->CreateBuffer(&vertex_desc,	nullptr, &vertex_staging_buffer); VERIFY;
		hr = device::get()->CreateBuffer(&index_desc,	nullptr, &index_staging_buffer); VERIFY;

		device::context()->CopyResource(vertex_staging_buffer,	vertex_buffer.Get());
		device::context()->CopyResource(index_staging_buffer,	index_buffer.Get());

		D3D11_MAPPED_SUBRESOURCE vertex_resource;
		hr = device::context()->Map(vertex_staging_buffer, 0, D3D11_MAP_READ, 0, &vertex_resource); VERIFY;
		auto* vertices = reinterpret_cast<const vertex*>(vertex_resource.pData);

		D3D11_MAPPED_SUBRESOURCE index_resource;
		hr = device::context()->Map(index_staging_buffer, 0, D3D11_MAP_READ, 0, &index_resource); VERIFY;
		auto* indices = reinterpret_cast<const uint32_t*>(index_resource.pData);

		
		uint32_t index_count = index_desc.ByteWidth / sizeof(uint32_t);

		for (uint32_t i = 0; i < index_count; i += 3) {
			triangle_mesh.push_back( {
				vertices[indices[i + 0]].position,
				vertices[indices[i + 1]].position,
				vertices[indices[i + 2]].position
			} );
		}

		device::context()->Unmap(vertex_staging_buffer, 0);
		device::context()->Unmap(index_staging_buffer, 0);
		vertex_staging_buffer->Release();
		index_staging_buffer->Release();

		unwrapped = true;
	}

	uint32_t mesh::ray_collision(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit) const {
		uint32_t hit = 0;

		for (const auto& triangle : peek_triangles()) {
			float3 int_point;
			float3 int_normal;

			if (triangle_intersection(triangle * model_transform, origin, ray, out_int_point ? &int_point : nullptr, out_int_normal ? &int_normal : nullptr)) {
				if (any_hit) { return 1; }

				if (out_int_point && out_int_normal) {
					if ((hit == 0) || (int_point - origin).mag_sq() < (*out_int_point - origin).mag_sq()) {
						*out_int_point = int_point;
						*out_int_normal = int_normal;
					}
				}
				hit++;
			}
		}

		return hit;
	}
	
	// Model 

	void model::create_shaders(std::string cso) {
		HRESULT hr{ S_OK };
		vs_cso = cso;

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
		if (transition != -2) {
			if (!animations.at(transition).update(elapsed_time)) { transition = -2; }
			else {
				transition_timer += elapsed_time;
				if (transition_timer > transition_duration) { sequence = transition; transition = -2; return; }
			}
		}

		if (sequence == -2) return;
		if (!animations.at(sequence).update(elapsed_time)) { sequence = transition ? transition : -2; transition = -2; }
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
			blend_animations(animations.at(sequence).get_keyframe(), animations.at(transition).get_keyframe(), transition_timer / transition_duration, temp_frame);
			return &temp_frame;
		}
		else { return &animations.at(sequence).get_keyframe(); }
	}

	// Primitives

	int primitive_detail = 12;
	void set_primitive_detail(int level) { primitive_detail = level; }

	model* create_cube		(float3 min, float3 max)		{ return new custom_cube(min, max);							}
	model* create_sphere	()								{ return new sphere		(primitive_detail);					}
	model* create_cylinder	()								{ return new cylinder	(primitive_detail);					}
	model* create_capsule	(float height, float radius)	{ return new capsule	(height, radius, primitive_detail); }
	model* create_quad		()								{ return new quad;											}

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