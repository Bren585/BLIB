#pragma once
#include "math.h"

namespace BLIB::debug::draw {
	struct vertex {
		float3 pos;
		color color;
	};

	void push_to_screen(vertex vertices[], unsigned int vertex_count);
	void uninit();
}