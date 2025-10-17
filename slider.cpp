#include "pch.h"

class any_slider {
protected:
	float elapsed;
	float duration;

	virtual void write() = 0;

public:
	any_slider(float duration) : elapsed(0), duration(duration) {}
	virtual ~any_slider() = default;

	bool finished() { return elapsed > duration; }
	bool update(float dt) { if (finished()) { return false; } elapsed += dt; write(); return !finished(); }
};

#define DEFINE_SLIDER(TYPE, NAME)																						\
class NAME : public any_slider {																						\
public:																													\
	using interpolator = std::function<TYPE(const TYPE&, const TYPE&, float)>;											\
private:																												\
	TYPE* target;																										\
	TYPE start;																											\
	TYPE end;																											\
    interpolator intp;																									\
public:																													\
	NAME(TYPE& start, TYPE end, float duration, interpolator intp)														\
		: target(&start), start(start), end(end), any_slider(duration), intp(intp) {}									\
	void write() { *target = intp(start, end, clamp(0, elapsed / duration, 1)); }										\
};																														\
																														\
void at(TYPE& start, TYPE end, float duration, slide_type intp_type) {													\
	NAME::interpolator intp;																							\
	switch (intp_type) {																								\
	default: intp = static_cast<TYPE(*)(const TYPE&, const TYPE&, float)>(lerp); break;									\
	}																													\
	sliders.push_back(std::make_unique<NAME>(start, end, duration, intp));												\
}

namespace slide {
	std::vector<std::unique_ptr<any_slider>> sliders;

	void update(float elapsed_time) {
		for (auto it = sliders.begin(); it != sliders.end(); ) {
			if (!it->get()->update(elapsed_time))	{ it = sliders.erase(it); }
			else									{ ++it; }
		}
	}

	DEFINE_SLIDER(float,		float_slider)
	DEFINE_SLIDER(float2,		float2_slider)
	DEFINE_SLIDER(float3,		float3_slider)
	DEFINE_SLIDER(float4,		float4_slider)
	DEFINE_SLIDER(transform,	transform_slider)
}