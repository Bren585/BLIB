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
};

std::map<font_name, font>	fonts			= std::map<font_name, font>();
string						fonts_filepath	= L"-1";

void font::write(string s, float2 pos, float2 size, color color) {
	float2 uv = { static_cast<float>(texture2d_desc.Width / 16.0f), static_cast<float>(texture2d_desc.Height / 16.0f) };
	float2 carry = 0;

	begin(color);
	render_settings{ sampler::POINT, vertex_shader(sprite::get_default_vs()), pixel_shader(DEFAULT_FLAT) }.set();
	for (const char c : s) {
		prerender(pos + carry, size, uv * float2((float)(c & 0x0F), (float)(c >> 4)), uv, 0, C_CC);
		carry.x += size.x;
	}
	end();
}

string text::get_filename_from_font(font_name f) {
	_ASSERT_EXPR(fonts_filepath != L"-1", L"Must Set Filepath");
	return fonts_filepath + font_filename[f];
}

float text::out(string s, float2 pos, float2 size, font_name font_name, color color, float2 align) {
	_ASSERT_EXPR(font_name >= 0 && font_name < COUNT_FONT, L"Illegal Font Value");

	pos -= float2{size.x * s.length(), size.y} * (align + float2(1)) * 0.5f;

	auto it = fonts.find(font_name);
	if (it != fonts.end()) {
		it->second.write(s, pos, size, color);
	}
	else {
		font newfont(font_name);
		newfont.write(s, pos, size, color);
		fonts.try_emplace(font_name, std::move(newfont));
	}
	return size.y;
}

void text::set_filepath(string path) { fonts_filepath = path; }