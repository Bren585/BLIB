#include "pch.h"
#include "materials.h"
#include "texture.h"
#include "canvas.h"
#include "object.h"

namespace BLIB {
	void material_texture_unpacked_orm::force_construct() {
		annotate("unpack_orm");

		RENDER_LOCK;
		std::unique_ptr<sprite> channels[3];
		float2 size = { 1, 1 };

		if (filenames[0] != "") { channels[0] = std::make_unique<sprite>(filenames[0]);	float2 channel_size = channels[0]->get_size(); if (channel_size > size) size = channel_size; }
		else					{ channels[0] = std::make_unique<sprite>(color{c.r, 0, 0}, float2{ 1 }); }

		if (filenames[1] != "")	{ channels[1] = std::make_unique<sprite>(filenames[1]); float2 channel_size = channels[1]->get_size(); if (channel_size > size) size = channel_size; }
		else					{ channels[1] = std::make_unique<sprite>(color{0, c.g, 0}, float2{ 1 }); }

		if (filenames[2] != "")	{ channels[2] = std::make_unique<sprite>(filenames[2]); float2 channel_size = channels[2]->get_size(); if (channel_size > size) size = channel_size; }
		else					{ channels[2] = std::make_unique<sprite>(color{0, 0, c.b}, float2{ 1 }); }

		canvas capture(size);
		capture.focus();

		render_settings rs{ pixel_shader{"orm_compression"} };
		rs &= sprite::default_rs();
		rs.set();

		for (int i = 0; i < 3; i++) { device::context()->PSSetShaderResources(i, 1, channels[i]->get_SRV()); }

		Microsoft::WRL::ComPtr<ID3D11Buffer> point_buffer, constant_buffer;

		make_point_buffer(point_buffer.GetAddressOf());
		update_point_buffer(point_buffer.Get(), size / 2, float2{ 1 }, C_CC, 0, float2{ 0 });

		make_constant_buffer(constant_buffer.GetAddressOf());
		update_constant_buffer(constant_buffer.GetAddressOf(), size, WHITE, size);

		draw_points(point_buffer.GetAddressOf());
		capture.unfocus();

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
		canvas capture(bump_map.get_size());
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