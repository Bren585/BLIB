#include "pch.h"
#include "materials.h"
#include "texture.h"
#include "canvas.h"
#include "object.h"

namespace BLIB {
	void material_texture_unpacked_orm::force_construct() {
		std::unique_ptr<sprite> channels[3];
		float2 size;

		if (filenames[0] == "") { channels[0] = std::make_unique<sprite>(filenames[0]);	float2 channel_size = channels[0]->get_size(); if (channel_size > size) size = channel_size; }
		else					{ channels[0] = std::make_unique<sprite>(color{c.r, 0, 0}, float2{ 1 }); }

		if (filenames[1] == "")	{ channels[1] = std::make_unique<sprite>(filenames[1]); float2 channel_size = channels[1]->get_size(); if (channel_size > size) size = channel_size; }
		else					{ channels[1] = std::make_unique<sprite>(color{0, c.g, 0}, float2{ 1 }); }

		if (filenames[2] == "")	{ channels[2] = std::make_unique<sprite>(filenames[2]); float2 channel_size = channels[2]->get_size(); if (channel_size > size) size = channel_size; }
		else					{ channels[2] = std::make_unique<sprite>(color{0, 0, c.b}, float2{ 1 }); }

		for (int i = 0; i < 3; i++) { device::context()->PSSetShaderResources(i, 1, channels[i]->get_SRV()); }

		canvas capture(size);
		capture.focus(); // set RTV

		Microsoft::WRL::ComPtr<ID3D11Buffer> quad_buffer;
		make_quad_buffer(quad_buffer.GetAddressOf());
	
		sprite::load_shader();
		render_settings rs{ pixel_shader{"orm_compression"}, vertex_shader(DEFAULT_FLAT) };
		rs.set();

		uint32_t stride{ sizeof(sprite::vertex) };
		uint32_t offset{ 0 };
		device::context()->IASetVertexBuffers(0, 1, quad_buffer.GetAddressOf(), &stride, &offset);
		device::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		device::context()->Draw(4, 0);

		data.reset(capture.peek_sprite()->clone());
	}

	void construct_pbr_from_phong(material* mat, color Ka, color Kd, color Ks) {
		auto& textures = mat->textures;
		if (!textures[texture_type::texture_map]) { textures[texture_type::texture_map].reset(new material_texture_dummy(Kd)); }
		
		float occlusion = (Ks.r + Ks.g + Ks.b) / 3.0f;
		float roughness = DEFAULT_ROUGHNESS;
		float metallic = (occlusion > 0.2f) ? 1.0f : 0.0f;

		textures[texture_type::ORM].reset(new material_texture_dummy({ occlusion, roughness, metallic }));
	}

	void construct_normal_from_bump(std::unique_ptr<sprite>& normal_map, string filename) {
		flat::object bump_map(filename);
		canvas capture(bump_map.size);
		capture.draw(&bump_map, render_settings{ pixel_shader{"bump_to_norm"} });
		normal_map.reset(capture.peek_sprite()->clone());
	}
}

CEREAL_REGISTER_TYPE(BLIB::material_texture_file)
CEREAL_REGISTER_TYPE(BLIB::material_texture_dummy)
CEREAL_REGISTER_TYPE(BLIB::material_texture_unpacked_orm)
CEREAL_REGISTER_POLYMORPHIC_RELATION(BLIB::material_texture, BLIB::material_texture_file)
CEREAL_REGISTER_POLYMORPHIC_RELATION(BLIB::material_texture, BLIB::material_texture_dummy)
CEREAL_REGISTER_POLYMORPHIC_RELATION(BLIB::material_texture, BLIB::material_texture_unpacked_orm)

CEREAL_REGISTER_TYPE(BLIB::material_texture_height)
CEREAL_REGISTER_POLYMORPHIC_RELATION(BLIB::material_texture,		BLIB::material_texture_height)
CEREAL_REGISTER_POLYMORPHIC_RELATION(BLIB::material_texture_file,	BLIB::material_texture_height)