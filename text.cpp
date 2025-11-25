#include "pch.h"
#include "text.h" 
#include "render_settings.h"
using namespace BLIB;
using namespace text;

string font_filename[COUNT_FONT] = {
	L"font0.png",
	L"font1_fixed.png",
	L"font2.png",
	L"font3.png",
	L"font6.png",
	L"font_test.png"
};

std::map<font_name, font>	fonts			= std::map<font_name, font>();
string						fonts_filepath	= L"-1";

void font::render_each(float2 pos, float2 scale, float2 pivot, float rotation, float2 tile_index, float2 tile_size) {
	float2 true_tile_size = get_size() * 0.0625f; // 1 / 16
	scale = tile_size / true_tile_size;
	float2 carry = { 0, 0 };
	(render_settings{ sampler::POINT, pixel_shader(DEFAULT_FLAT) } & sprite::default_rs()).set();
	for (const char c : buffer) {
		prerender(pos + carry, scale, pivot, rotation, float2((float)(c & 0x0F), (float)(c >> 4)), true_tile_size);
		carry.x += tile_size.x;
	}
}

string text::get_filename_from_font(font_name f) {
	_ASSERT_EXPR(fonts_filepath != L"-1", L"Must Set Filepath");
	return fonts_filepath + font_filename[f];
}

float text::out(string s, float2 pos, float2 size, font_name font_name, color color, float2 align) {
	_ASSERT_EXPR(font_name >= 0 && font_name < COUNT_FONT, L"Illegal Font Value");

	pos -= float2{size.x * s.length(), size.y} * (align + float2(1, 1)) * 0.5f;

	auto it = fonts.find(font_name);
	if (it != fonts.end()) {
		it->second.buffer = s;
		it->second.render(pos, float2{ 1 }, C_BL, 0, color, float2{ 0 }, size);
	}
	else {
		font newfont(font_name);
		newfont.buffer = s;
		newfont.render(pos, float2{ 1 }, C_BL, 0, color, float2{ 0 }, size);
		fonts.try_emplace(font_name, std::move(newfont));
	}
	return size.y;
}

void text::set_filepath(string path) { fonts_filepath = path; }