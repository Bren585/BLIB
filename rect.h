#pragma once
#include "shape.h"
namespace BLIB::debug::draw {
	inline void rect(float2 pos, float2 size, color c, float angle = 0) {
		constexpr unsigned int vertex_count = 5;
		vertex vertices[vertex_count] = { 
			{ { pos + size							.rotate(angle),	0 }, c },
			{ { pos + (size * float2{1.0f, -1.0f})	.rotate(angle),	0 }, c },
			{ { pos + (-size)						.rotate(angle),	0 }, c },
			{ { pos + (size * float2{-1.0f, 1.0f})	.rotate(angle),	0 }, c },
			{ { pos + size							.rotate(angle),	0 }, c },
		};
		push_to_screen(vertices, vertex_count);
	}
}