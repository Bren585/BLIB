#pragma once
#include "cereal.h"
#include "sprite.h"
#include "device.h"
#include <array>

#define DEFAULT_TEXTURE_MAP		WHITE
#define DEFAULT_NORMAL_MAP		color {0.5f, 0.5f, 1.0f}
#define DEFAULT_OCCLUSION		1.0f
#define DEFAULT_ROUGHNESS		0.5f
#define DEFAULT_METALLIC		0.0f
#define DEFAULT_ORM_MAP			color {DEFAULT_OCCLUSION, DEFAULT_ROUGHNESS, DEFAULT_METALLIC}
#define DEFAULT_EMISSION_MAP	COLORLESS

namespace BLIB {
	struct phong {
		uint64_t	unique_id{ 0 };
		string		name;

		color Ka{ 0.2f, 0.2f, 0.2f, 1.0f };
		color Kd{ 0.8f, 0.8f, 0.8f, 1.0f };
		color Ks{ 1.0f, 1.0f, 1.0f, 1.0f };

		string texture_filenames[4];

		SERIALIZE(unique_id, name, Ka, Kd, Ks, texture_filenames)
	};

	struct material_texture {
		std::unique_ptr<sprite> data{ nullptr };
		material_texture() = default;
		material_texture(const material_texture&) = delete;
		material_texture(material_texture&&) = default;
		void construct() { if (!data) force_construct(); }
		virtual void force_construct() {}
		virtual ~material_texture() = default;
		SERIALIZE_EMPTY()
	};

	struct material_texture_file : material_texture {
		string filename;
		material_texture_file() = default;
		material_texture_file(string filename) : filename(filename) {}
		material_texture_file(const material_texture_file&) = delete;
		material_texture_file(material_texture_file&&) = default;
		void force_construct() override { if (!data) data = std::make_unique<sprite>(sprite::load_texture, filename); }
		SERIALIZE_BASE(material_texture, filename)
	};

	void construct_normal_from_bump(std::unique_ptr<sprite>& normal_map, string filename);

	struct material_texture_height : material_texture_file {
		material_texture_height() = default;
		material_texture_height(string filename) : material_texture_file(filename) {}
		material_texture_height(const material_texture_height&) = delete;
		material_texture_height(material_texture_height&&) = default;
		void force_construct() override { construct_normal_from_bump(data, filename); }
		SERIALIZE_EMPTY_BASE(material_texture_file)
	};

	struct material_texture_dummy : material_texture {
		color c;
		material_texture_dummy() = default;
		material_texture_dummy(color c) : c(c) {}
		material_texture_dummy(const material_texture_dummy&) = delete;
		material_texture_dummy(material_texture_dummy&&) = default;
		void force_construct() override { data = std::make_unique<sprite>(c, float2{ 1.0f }); }
		SERIALIZE_BASE(material_texture, c)
	};

	struct material_texture_unpacked_orm : material_texture {
		color c{ 0, 0, 0 };
		string filenames[3]{};
		material_texture_unpacked_orm() = default;
		material_texture_unpacked_orm(color c, string files[3]) : c(c) { for (int i = 0; i < 3; i++) { filenames[i] = files[i]; } }
		material_texture_unpacked_orm(const material_texture_unpacked_orm&) = delete;
		material_texture_unpacked_orm(material_texture_unpacked_orm&&) = default;
		void force_construct() override;
		SERIALIZE_BASE(material_texture, c, filenames[0], filenames[1], filenames[2])
	};

	enum texture_type {
		texture_map,
		normal_map,
		ORM,
		emissive,

		texture_count,
	};

	inline std::unique_ptr<material_texture_dummy> create_default_material(texture_type type) {
		switch (type) {
		case normal_map:	return std::make_unique<material_texture_dummy>(DEFAULT_NORMAL_MAP		);
		case ORM:			return std::make_unique<material_texture_dummy>(DEFAULT_ORM_MAP			);
		case emissive:		return std::make_unique<material_texture_dummy>(DEFAULT_EMISSION_MAP	);
		default:			return std::make_unique<material_texture_dummy>(DEFAULT_TEXTURE_MAP		); // texture_map
		}
	}

	struct material {

		uint64_t unique_id{ 0 };
		string name;
		std::array<std::unique_ptr<material_texture>, texture_count> textures = { nullptr };

		material() = default;
		material(material&&) = default;
		material(const material& o) {
			unique_id = o.unique_id;
			name = o.name;
			for (int i = 0; i < texture_count; i++) {
				textures[i] = std::make_unique<material_texture>();
				textures[i]->data.reset(o.textures[i]->data->clone());
			}
		}

		void construct() {
			for (int t = 0; t < texture_count; t++) {
				auto& tex = textures[t];
				if (!tex) { tex = create_default_material(texture_type(t)); }
				tex->construct();
			}
		}

		void force_construct() {
			for (int t = 0; t < texture_count; t++) {
				auto& tex = textures[t];
				if (!tex) { tex = create_default_material(texture_type(t)); }
				tex->force_construct();
			}
		}

		void bind(int slot) const {
			for (int i = 0; i < 4; i++) {
				device::context()->PSSetShaderResources(slot + i, 1, textures[i]->data->get_SRV());
			}
		}

		SERIALIZE(unique_id, name, textures)
	};

	void construct_pbr_from_phong(material* mat, color Ka, color Kd, color Ks);

}