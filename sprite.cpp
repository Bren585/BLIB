#include "pch.h"
#include "sprite.h"
#include "shader.h"
#include "texture.h"

using namespace BLIB;

bool	sprite::y_invert = true;
float2	sprite::viewport = {};
string	sprite::filepath = "-1";

void sprite::create_buffer() {
	vertex vertices[]
	{
		{ { -1.0, +1.0, 0 }, { 1, 1, 1, 1 }, {0, 0} },
		{ { +1.0, +1.0, 0 }, { 1, 1, 1, 1 }, {1, 0} },
		{ { -1.0, -1.0, 0 }, { 1, 1, 1, 1 }, {0, 1} },
		{ { +1.0, -1.0, 0 }, { 1, 1, 1, 1 }, {1, 1} },
	};

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(vertices);
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresource_data{};
	subresource_data.pSysMem = vertices;
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	HRESULT hr = device::get()->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.GetAddressOf()); VERIFY;
}

sprite::sprite(flags flags, const string& filename) {
	HRESULT hr{ S_OK };

	if (flags & make_buffer) { create_buffer(); }

	D3D11_INPUT_ELEMENT_DESC input_element_desc[]{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	shader::load_flat(vs_cso, input_element_desc, _countof(input_element_desc));

	if (flags & load_texture) {
		string full_filepath = filename;
		if (!(flags & full_filename)) { _ASSERT_EXPR_A(filepath != "-1", "Must Set Filepath"); full_filepath = filepath + filename; }
		hr = texture::load_file(full_filepath, shader_resource_view.GetAddressOf(), &texture2d_desc); VERIFY;
	}
	else {
		texture2d_desc = D3D11_TEXTURE2D_DESC{};
		shader_resource_view = nullptr;
	}
}

sprite::sprite(color c, float2 size) : sprite(make_buffer) {
	texture::make_dummy(shader_resource_view.GetAddressOf(), c, size);
	resize(size);
}

sprite* sprite::clone() const {
	RENDER_LOCK;
	sprite* spr = new sprite(make_buffer);

	if (shader_resource_view.Get()) {
		HRESULT hr = { S_OK };

		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		shader_resource_view->GetResource(resource.GetAddressOf());
		hr = resource.As(&texture); VERIFY;
		texture->GetDesc(&spr->texture2d_desc);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> new_texture;
		hr = device::get()->CreateTexture2D(&spr->texture2d_desc, nullptr, &new_texture); VERIFY;
		device::context()->CopyResource(new_texture.Get(), texture.Get());
		hr = device::get()->CreateShaderResourceView(new_texture.Get(), nullptr, spr->shader_resource_view.GetAddressOf()); VERIFY;
	} 
	else {
		spr->texture2d_desc = texture2d_desc;
		spr->shader_resource_view = nullptr;
	}

	return spr;
}

bool sprite::create_vertices(vertex* vertex_out, float2 pos, float2 size, float2 tpos, float2 tsize, float angle, float2 center, color color) {
	//D3D11_VIEWPORT dx_viewport{};
	//UINT num_viewports{ 1 };
	//device::context()->RSGetViewports(&num_viewports, &dx_viewport);

	if (y_invert) { pos.y = viewport.y - pos.y; size.y *= -1; }

	bool xflip = size.x < 0;
	bool yflip = size.y < 0;

	if (xflip) { pos.x += size.x; size.x = -size.x; }
	if (yflip) { pos.y += size.y; size.y = -size.y; }

	if (
		pos.x + size.x < 0 ||
		pos.y + size.y < 0 ||
		pos.x > viewport.x ||
		pos.y > viewport.y
	) return false;

	float2 corner[4] = {
		0,
		size * float2(1, 0),
		size * float2(0, 1),
		size
	};

	//                    [-1 1] -> [0 1]
	float2 pivot = ((center + float2{1}) * 0.5f) * size;

	for (int i = 0; i < 4; i++) {
		corner[i] = pos + pivot + (corner[i] - pivot).rotate(y_invert ? -angle : angle);
		corner[i].x = 2.0f * corner[i].x / viewport.x - 1.0f;
		corner[i].y = 1.0f - 2.0f * corner[i].y / viewport.y;

		vertex_out[i].position = float3(corner[i], 0);
		vertex_out[i].color = color;
	}

	tpos.x /= texture2d_desc.Width;
	tpos.y /= texture2d_desc.Height;
	tsize.x = tsize.x > 0 ? tsize.x / texture2d_desc.Width  : 1;
	tsize.y = tsize.y > 0 ? tsize.y / texture2d_desc.Height : 1;

	if (y_invert) { yflip = !yflip; }

	float u[2] = { tpos.x, tpos.x + tsize.x };
	float v[2] = { tpos.y, tpos.y + tsize.y };

	vertex_out[0].texcoord = { u[ xflip],	v[ yflip] };
	vertex_out[1].texcoord = { u[!xflip],	v[ yflip] };
	vertex_out[2].texcoord = { u[ xflip],	v[!yflip] };
	vertex_out[3].texcoord = { u[!xflip],	v[!yflip] };

	return true;
}

void sprite::render(
	float2 pos, float2 size, 
	float2 tpos, float2 tsize, 
	float  angle, 
	float2 center,
	color  color
) {

	HRESULT hr{ S_OK };
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	bool sucess = false;

	hr = device::context()->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource); VERIFY;
	vertex* vertices{ reinterpret_cast<vertex*>(mapped_subresource.pData) };
	if (vertices) { sucess = create_vertices(vertices, pos, size, tpos, tsize, angle, center, color); }
	device::context()->Unmap(vertex_buffer.Get(), 0);
	if (!sucess) return;

	UINT stride{ sizeof(vertex) };
	UINT offset{ 0 };

	device::context()->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);

	device::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	shader::set_vs(vs_cso);

	device::context()->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());
	device::context()->Draw(4, 0);
}