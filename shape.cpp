#include "pch.h"
#include "shape.h"
#include "shader.h"
#include "window.h"

namespace BLIB::debug::draw {

	static bool ready = false;
	static std::map<int, Microsoft::WRL::ComPtr<ID3D11Buffer>> vertex_buffers;
	//static Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;

	void init() {
		if (ready) return;
		D3D11_INPUT_ELEMENT_DESC input_element_desc[2] = {
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		shader::load_vs("debug_shapes", input_element_desc, 2);
		shader::load_ps("debug_shapes");

		//D3D11_BUFFER_DESC desc = {};
		//desc.ByteWidth = sizeof(float4x4);
		//desc.Usage = D3D11_USAGE_DEFAULT;
		//desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		//device::get()->CreateBuffer(&desc, nullptr, constant_buffer.ReleaseAndGetAddressOf());

		ready = true;
	}

	ID3D11Buffer* get_buffer(int vertex_count) {
		auto check = vertex_buffers.try_emplace(vertex_count);
		auto& vertex_buffer = check.first->second;
		if (check.second) {
			D3D11_BUFFER_DESC desc = {};
			desc.ByteWidth = sizeof(vertex) * vertex_count;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			device::get()->CreateBuffer(&desc, nullptr, vertex_buffer.ReleaseAndGetAddressOf());
		}
		return vertex_buffer.Get();
	}

	void push_to_screen(vertex vertices[], unsigned int vertex_count) {
		init();

		for (unsigned int i = 0; i < vertex_count; i++) {
			vertices[i].pos = { vertices[i].pos.xy() / (window::size() * 0.5f) - float2{ 1.0f }, vertices[i].pos.z };
		}

		RENDER_LOCK;
		ID3D11Buffer* vertex_buffer = get_buffer(vertex_count);
		D3D11_MAPPED_SUBRESOURCE mapped;
		device::context()->Map(vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		memcpy(mapped.pData, vertices, sizeof(vertex) * vertex_count);
		device::context()->Unmap(vertex_buffer, 0);

		UINT stride = sizeof(vertex);
		UINT offset = 0;

		device::context()->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
		device::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

		shader::set_ps("debug_shapes");
		shader::set_vs("debug_shapes");

		//const camera* cam = viewer::get_active();
		//float4x4 view_proj;
		//DirectX::XMStoreFloat4x4(&view_proj, cam ? DirectX::XMMatrixTranspose(cam->get_view_projection()) : DirectX::XMMatrixIdentity());

		//device::context()->UpdateSubresource(constant_buffer.Get(), 0, nullptr, &view_proj, 0, 0);
		//device::context()->VSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

		device::context()->Draw(vertex_count, 0);
	}

}