#pragma once
#include "shape.h"

namespace BLIB::debug::draw {
	inline void line(float3 a, float3 b, color c) {
		constexpr unsigned int vertex_count = 2;
		vertex vertices[vertex_count] = { {a, c}, {b, c} };
		push_to_screen(vertices, vertex_count);
	}

	inline void line(float2 a, float2 b, color c) {
		line({ a, 0 }, { b, 0 }, c);
	}
}