#include "pch.h"
#include "sprite_batch.h"
#include "shader.h"
#include "texture.h"

using namespace BLIB;

sprite_batch::sprite_batch(const string& filename, size_t max_sprites, flags flags) : sprite(flags, filename), max_vertices(max_sprites * 6) {
	HRESULT hr{ S_OK };

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = (UINT)(sizeof(vertex) * max_vertices);
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	hr = device::get()->CreateBuffer(&buffer_desc, NULL, vertex_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

sprite_batch::sprite_batch(sprite_batch&& o) noexcept : sprite(clone_flags), max_vertices(o.max_vertices) {
	vertices				= std::move(o.vertices				);
	vertex_buffer			= std::move(o.vertex_buffer			);
	shader_resource_view	= std::move(o.shader_resource_view	);
	texture2d_desc			= o.texture2d_desc;
}

void sprite_batch::prerender(float2 pos, float2 scale, float2 pivot, float rotation, float2 tile_index, float2 tile_size) {
	vertex& point		= vertices.emplace_back();

	point.position		= pos;
	point.size			= scale * tile_size;
	point.pivot			= pivot;
	point.rotation		= rotation;
	point.viewport		= get_viewport();
	point.y_invert		= get_y_invert();
	point.tile_size		= tile_size;
	point.tile_index	= tile_index;
	point.texture_size	= get_size();
}

void sprite_batch::begin(color color) {
	vertices.clear();
	update_constant_buffer(color);
}

void sprite_batch::end() {
	HRESULT hr{ S_OK };
	D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
	hr = device::context()->Map(vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource); VERIFY;
	size_t vertex_count = vertices.size();
	_ASSERT_EXPR(max_vertices >= vertex_count, "Buffer overflow");
	vertex* data{ reinterpret_cast<vertex*>(mapped_subresource.pData) };
	if (data != nullptr) {
		const vertex* p = vertices.data();
		memcpy_s(data, max_vertices * sizeof(vertex), p, vertex_count * sizeof(vertex));
	}
	device::context()->Unmap(vertex_buffer.Get(), 0);

	device::context()->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());
	draw_points(vertex_buffer.GetAddressOf(), static_cast<uint>(vertex_count));
}