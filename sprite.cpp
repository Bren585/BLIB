#include "pch.h"
#include "sprite.h"
#include "shader.h"
#include "texture.h"

using namespace BLIB;

void BLIB::make_point_buffer(ID3D11Buffer** out) {
	sprite::vertex point;
	point.position		= { 0.5f, 0.5f };
	point.size			= { 1, 1 };
	point.pivot			= { 0, 0 };
	point.rotation		= 0;
	point.viewport		= sprite::get_viewport();
	point.y_invert		= sprite::get_y_invert();
	point.tile_index	= { 0, 0 };
	point.tile_size		= { 1, 1 };

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth				= sizeof(sprite::vertex);
	desc.Usage					= D3D11_USAGE_DYNAMIC;
	desc.BindFlags				= D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags				= 0;
	desc.StructureByteStride	= 0;

	D3D11_SUBRESOURCE_DATA subresource_data{};
	subresource_data.pSysMem = &point;
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	HRESULT hr = device::get()->CreateBuffer(&desc, &subresource_data, out); VERIFY;
}

void BLIB::update_point_buffer(ID3D11Buffer* point_buffer, float2 pos, float2 scale, float2 pivot, float rotation, float2 tile_size, float2 tile_index, float2 texture_size) {
	HRESULT hr{ S_OK };
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	hr = device::context()->Map(point_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource); VERIFY;
	sprite::vertex* point{ reinterpret_cast<sprite::vertex*>(mapped_subresource.pData) };
	point->position		= pos;
	point->size			= tile_size * scale;
	point->pivot		= pivot;
	point->rotation		= rotation;
	point->viewport		= sprite::get_viewport();
	point->y_invert		= sprite::get_y_invert();
	point->tile_index	= tile_index;
	point->tile_size	= tile_size;
	point->texture_size = texture_size;
	device::context()->Unmap(point_buffer, 0);
}

void BLIB::draw_points(ID3D11Buffer* const* vertex_buffer_adr, uint count) {
	uint stride{ sizeof(sprite::vertex) };
	uint offset{ 0 };
	device::context()->IASetVertexBuffers(0, 1, vertex_buffer_adr, &stride, &offset);
	device::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	device::context()->Draw(count, 0);
}

bool	sprite::y_invert = true;
float2	sprite::viewport = float2(1);
string	sprite::filepath = "-1";

void sprite::create_vertex_buffer() {
	if (vertex_buffer) return;
	make_point_buffer(vertex_buffer.GetAddressOf());
}

void sprite::create_constant_buffer() {
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	HRESULT hr = device::get()->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf()); VERIFY;
}

void sprite::update_vertex_buffer(float2 pos, float2 scale, float2 pivot, float rotation, float2 tile_size, float2 tile_index) {
	update_point_buffer(vertex_buffer.Get(), pos, scale, pivot, rotation, tile_size, tile_index, get_size());
}

void sprite::update_constant_buffer(color color) {
	constants data;
	data.color = color;
	device::context()->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
	device::context()->PSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());
}

void sprite::load_shader(string vs) {
	D3D11_INPUT_ELEMENT_DESC input_element_desc[]{
		{ "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	2, DXGI_FORMAT_R32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	3, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	4, DXGI_FORMAT_R32_UINT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	5, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	6, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	7, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	shader::load_flat(vs, input_element_desc, _countof(input_element_desc));
}

void sprite::load_file(const string& filename, bool full_filepath) {
	if (shader_resource_view) return;
	string full_filename = filename;
	if (!full_filepath) { _ASSERT_EXPR_A(filepath != "-1", "Must Set Filepath"); full_filename = filepath + filename; }
	texture::load_file(full_filename, shader_resource_view.GetAddressOf(), &texture2d_desc);
}

sprite::sprite(flags flags, const string& filename) {
	if (flags & make_vbuffer	) { create_vertex_buffer();							}
	if (flags & make_cbuffer	) { create_constant_buffer();						}
	if (flags & load_shaders	) { load_shader(SPRITE_VS);							}
	if (flags & load_texture	) { load_file(filename, (flags & full_filename));	} else { resize(float2{1}); } // Make sure size is never 0
}

sprite::sprite(color c, float2 size) : sprite(dummy_flags) {
	texture::make_dummy(shader_resource_view.GetAddressOf(), c, size);
	resize(size);
}

sprite* sprite::clone() const {
	RENDER_LOCK;
	sprite* spr = new sprite(clone_flags);

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

void sprite::render(
	float2	pos, 
	float2	scale, 
	float2	pivot, 
	float	rotation, 
	color	color, 
	float2	tile_index,
	float2	tile_size
) {
	update_vertex_buffer(pos, scale, pivot, rotation, tile_size, tile_index);
	update_constant_buffer(color);
	device::context()->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());
	draw_points(vertex_buffer.GetAddressOf());
}