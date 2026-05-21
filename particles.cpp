#include "pch.h"
#include "particles.h"
#include "constant_buffer_indices.h"

namespace BLIB {

	void particles::com_setup() {
		input_element_desc = {
			{"POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",	2, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		create_shaders("billboard");
		shader::load_gs("billboard");

		material mat;
		mat.name = "particle_material";
		mat.unique_id = 0;
		mat.construct();
		materials.emplace(mat.unique_id, std::move(mat));

		/* Vertex Buffer */ {
			HRESULT hr{ S_OK };

			D3D11_BUFFER_DESC buffer_desc{};
			buffer_desc.ByteWidth			= (uint)(sizeof(point) * MAX_MAX_PARTICLES);
			buffer_desc.Usage				= D3D11_USAGE_DYNAMIC;
			buffer_desc.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
			buffer_desc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
			buffer_desc.MiscFlags			= 0;
			buffer_desc.StructureByteStride = 0;
			hr = device::get()->CreateBuffer(&buffer_desc, NULL, vertex_buffer.GetAddressOf()); VERIFY;
		}
	}

	void particles::render(const float4x4& world, const color& material_color) const {
		constants data{ world, material_color };
		device::context()->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
		device::context()->GSSetConstantBuffers(FULL_VS_CB, 1, constant_buffer.GetAddressOf());
		materials.at(0).bind(0);

		uint32_t stride{ sizeof(point) };
		uint32_t offset{ 0 };
		device::context()->IASetVertexBuffers(0, 1, get_vertices(), &stride, &offset);
		device::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		
		device::context()->Draw(static_cast<uint>(particle_buffer.size()), 0);
	}

	void particles::_update(float elapsed_time) {
		point_buffer.clear();
		if (active && particle_buffer.size() < max_particles) { emit(elapsed_time); }
		for (auto pi = particle_buffer.begin(); pi != particle_buffer.end();) {
			auto& particle = *pi;
			particle.lifetime += elapsed_time;
			particle.animator.update(elapsed_time);
			if (particle.lifetime > particle.max_lifetime) {
				pi = particle_buffer.erase(pi);
				continue;
			}
			else {
				pi++;
			}
			update_each(elapsed_time, particle);
			point& p = point_buffer.emplace_back(particle.p);
			p.uv_index = particle.animator.get_frame();
			p.size = particle.p.size * particle.scale;
		}

		RENDER_LOCK;

		HRESULT hr{ S_OK };

		D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
		hr = device::context()->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource); VERIFY;
		if (!FAILED(hr)) {
			volatile size_t test = point_buffer.capacity();
			size_t vertex_count = point_buffer.size();
			point* data{ reinterpret_cast<point*>(mapped_subresource.pData) };
			if (data != nullptr) {
				const point* p = point_buffer.data();
				memcpy_s(data, MAX_MAX_PARTICLES * sizeof(point), p, vertex_count * sizeof(point));
			}
		}
		device::context()->Unmap(vertex_buffer.Get(), 0);
	}


}