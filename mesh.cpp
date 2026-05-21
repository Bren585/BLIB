#include "pch.h"
#include "mesh.h"
#include "constant_buffer_indices.h"

namespace BLIB {

	void mesh::create_buffers() {
		HRESULT hr{ S_OK };

		D3D11_BUFFER_DESC buffer_desc{};
		D3D11_SUBRESOURCE_DATA subresource_data{};

		buffer_desc.ByteWidth = static_cast<uint>(sizeof(vertex) * vertices.size());
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;

		subresource_data.pSysMem = vertices.data();
		subresource_data.SysMemPitch = 0;
		subresource_data.SysMemSlicePitch = 0;

		hr = device::get()->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.ReleaseAndGetAddressOf()); VERIFY;

		buffer_desc.ByteWidth = static_cast<uint>(sizeof(uint32_t) * indices.size());
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		subresource_data.pSysMem = indices.data();

		hr = device::get()->CreateBuffer(&buffer_desc, &subresource_data, index_buffer.ReleaseAndGetAddressOf()); VERIFY;

#ifdef SKIN_GPU

		D3D11_BUFFER_DESC constant_buffer_desc{};
		constant_buffer_desc.ByteWidth = static_cast<uint>(sizeof(float4x4) * (bind_pose.bones.size() == 0 ? 1 : bind_pose.bones.size()));
		constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constant_buffer_desc.CPUAccessFlags = 0;
		constant_buffer_desc.MiscFlags = 0;
		constant_buffer_desc.StructureByteStride = 0;
		hr = device::get()->CreateBuffer(&constant_buffer_desc, nullptr, bone_buffer.ReleaseAndGetAddressOf()); VERIFY;

		//struct bone_inf { float weights[4]; uint indices[4]; };
		//std::vector<bone_inf> bone_data;
		//bone_data.resize(vertices.size());
		//for (uint i = 0; i < vertices.size(); i++) {
		//	bone_inf& inf = bone_data[i];
		//	const vertex& v = vertices[i];
		//	for (int j = 0; j < MAX_BONE_INF; j++) {
		//		inf.weights[j] = v.bone_weights[j];
		//		inf.indices[j] = v.bone_indices[j];
		//	}
		//}
		//
		//D3D11_BUFFER_DESC structured_buffer_desc{};
		//structured_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		//structured_buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		//structured_buffer_desc.CPUAccessFlags = 0;
		//structured_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		//structured_buffer_desc.StructureByteStride = sizeof(bone_inf);
		//structured_buffer_desc.ByteWidth = static_cast<uint>(sizeof(bone_inf) * vertices.size());
		//subresource_data.pSysMem = bone_data.data();
		//
		//hr = device::get()->CreateBuffer(&structured_buffer_desc, &subresource_data, bone_inf_buffer.GetAddressOf()); VERIFY;
		//
		//D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		//srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		//srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		//srv_desc.Buffer.FirstElement = 0;
		//srv_desc.Buffer.NumElements = static_cast<uint>(vertices.size());
		//hr = device::get()->CreateShaderResourceView(bone_inf_buffer.Get(), &srv_desc, bone_inf_srv.GetAddressOf()); VERIFY;

#endif
	}

	void mesh::update_buffers(const std::vector<vertex>& vertices) const {
		RENDER_LOCK;
		D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
		HRESULT hr = device::context()->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource); VERIFY;
		memcpy(mapped_subresource.pData, vertices.data(), vertices.size() * sizeof(vertex));
		device::context()->Unmap(vertex_buffer.Get(), 0);
	}

	void mesh::update_bone_buffer(std::vector<float4x4>& bone_transforms) const {
		assert(bone_transforms.size() == bind_pose.bones.size());
		RENDER_LOCK;
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

}