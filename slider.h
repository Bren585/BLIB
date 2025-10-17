#pragma once
#include "math.h"

// Still needs lifetime safety... probably use some kind of subscriber system so that classes can unsub when they die

#define DECLARE_SLIDER(TYPE) void at(TYPE&, TYPE, float, slide_type = slide_lerp);

namespace slide {
	enum slide_type {
		slide_lerp,
		slide_ease,
	};

	class slider;

	void update(float elapsed_time);
	
	DECLARE_SLIDER(float)
	DECLARE_SLIDER(float2)
	DECLARE_SLIDER(float3)
	DECLARE_SLIDER(float4)
	DECLARE_SLIDER(transform)
}

