#pragma once
#include "shape.h"
namespace BLIB::debug::draw {
	inline void circle(float2 pos, float r, color c, int edges = 12) {
		assert(edges > 3);
		unsigned int vertex_count = edges + 1;
		vertex* vertices = new vertex[vertex_count];
		const float step = PI2 / edges;
		float angle = 0;
		for (int vi = 0; vi < edges; vi++) {
			vertices[vi] = { { pos + r * float2{cos(angle), sin(angle)}, 0 }, c };
			angle += step;
		}
		vertices[edges] = vertices[0];
		push_to_screen(vertices, vertex_count);
		delete[] vertices;
	}
}