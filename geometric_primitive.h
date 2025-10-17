#pragma once
#include "model.h"
#include "texture.h"

namespace BLIB {

	class geometric_primitive : public model, public mesh { 
	protected:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
		virtual geometric_primitive* clone_impl() const override { return new geometric_primitive(*this); }
		virtual void render(const float4x4& world, const color& material_color) const override;

	public:
		geometric_primitive();
		virtual ~geometric_primitive() = default;

		inline void load_texture(const wchar_t* filename, D3D11_TEXTURE2D_DESC* desc)			{ texture::load_file(filename, texture.ReleaseAndGetAddressOf(), desc); }
		inline void copy_texture(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& SRV)	{ SRV.CopyTo(texture.ReleaseAndGetAddressOf()); }

		inline ID3D11ShaderResourceView** SRV_copy_dest() { return texture.ReleaseAndGetAddressOf(); }

		inline float3 get_size() const override { return mesh::size(); }
		inline const std::vector<triangle>& peek_triangles() const override { return mesh::peek_triangles(); }
		inline uint32_t ray_collision(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const override { return mesh::ray_collision(model_transform, origin, ray, out_int_point, out_int_normal, any_hit); }
	
		void create_subsets();
	};

	class quad : public geometric_primitive {
	public:
		quad();
	};

	class cube : public geometric_primitive {
	public:
		cube();
	};

	class custom_cube : public geometric_primitive {
	public:
		custom_cube(float3 min, float3 max);
	};

	class cylinder : public geometric_primitive {
	public:
		cylinder() : cylinder(12) {}
		cylinder(int edges);
	};

	class sphere : public geometric_primitive {
	public:
		sphere() : sphere(12) {}
		sphere(int edges);
	};

	class capsule : public geometric_primitive {
	public:
		capsule() : capsule(0.5f, 0.5f, 12) {}
		capsule(int edges) : capsule(0.5f, 0.5f, edges) {}
		capsule(float radius) : capsule(radius, radius, 12) {}
		capsule(float height, float radius) : capsule(height, radius, 12) {}
		capsule(float height, float radius, int edges);
	};

}