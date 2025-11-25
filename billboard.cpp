#include "pch.h"
#include "billboard.h"

namespace BLIB {

	struct point {
		float4 position = float4{ 0 };
		float2 size		= float2{ 1 };
	};

	billboard::billboard(float2 size) {
		input_element_desc = {
			{"POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		create_shaders("billboard");
		shader::load_gs("billboard");

		material mat;
		mat.name = "billboard_material";
		mat.unique_id = 0;
		mat.construct();
		materials.emplace(mat.unique_id, std::move(mat));

		/* Vertex Buffer */ {
			point p;
			p.size = size;

			HRESULT hr{ S_OK };

			D3D11_BUFFER_DESC desc{};
			D3D11_SUBRESOURCE_DATA subresource_data{};

			desc.ByteWidth = static_cast<uint>(sizeof(point));
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			subresource_data.pSysMem = &p;
			subresource_data.SysMemPitch = 0;
			subresource_data.SysMemSlicePitch = 0;

			hr = device::get()->CreateBuffer(&desc, &subresource_data, vertex_buffer.ReleaseAndGetAddressOf()); VERIFY;
		}
	}

	void billboard::render(const float4x4& world, const color& material_color) const {

		uint32_t stride{ sizeof(point) };
		uint32_t offset{ 0 };

		device::context()->IASetVertexBuffers(0, 1, get_vertices(), &stride, &offset);
		device::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		transform static_world(world);
		static_world.set_qtn(quaternion::identity());

		constants data{ static_world, material_color };
		device::context()->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
		device::context()->GSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

		materials.at(0).bind(0);

		device::context()->Draw(1, 0);
	}

}