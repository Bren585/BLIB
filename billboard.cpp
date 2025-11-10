#include "pch.h"
#include "billboard.h"
//#include "camera.h"

using namespace BLIB;

void billboard::set_texture(const string filename) {
	float3 scale;
	load_texture(get_model(), filename, texture_map, &scale);
	set_scl(scale);
}

void billboard::set_texture(const sprite* spr) {
	float3 scale;
	copy_texture(get_model(), spr, &scale);
	set_scl(scale);
}

void billboard::set_dynamic(bool on) { dynamic = on; }

void billboard::_basic_update(float elapsed_time) {
	//if (dynamic) { set_ang(to_euler(-camera::get_active()->get_facing())); }
}

void billboard::_render(render_settings rs) const {
	full::object::_render(rs);
}