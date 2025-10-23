#pragma once

#include "model.h"
#include "string.h"
#include "math.h"

namespace BLIB {

	class custom_cube;

	class static_mesh : public model, protected mesh {
	private:
		std::unique_ptr<model> bounding_box = nullptr;

	protected:
		virtual static_mesh* clone_impl() const override { return new static_mesh(*this); }
		void render(const float4x4& world, const color& material_color) const override;

	public:
		static_mesh(const string& obj_filename) : static_mesh(obj_filename, false) {}
		static_mesh(const string& obj_filename, bool uv_inverse);
		static_mesh(const static_mesh& o) : model(o), mesh(o), bounding_box(nullptr) {}
		virtual ~static_mesh();

		void render_bounds(const float4x4& world, const color& material_color);

		inline float3 get_size() const override { return mesh::size(); }
		inline const std::vector<triangle>& peek_triangles() const override { return mesh::peek_triangles(); }
		inline uint32_t ray_collision(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit = false) const override { return mesh::ray_collision(model_transform, origin, ray, out_int_point, out_int_normal, any_hit); }
	};

}