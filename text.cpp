#include "pch.h"
#include "text.h" 
#include "render_settings.h"
#include <fstream>
using namespace BLIB;
using namespace text;

//string font_filename[COUNT_FONT] = {
//	L"font0.png",
//	L"font1_fixed.png",
//	L"font2.png",
//	L"font3.png",
//	L"font6.png",
//	L"font_test.png"
//};

std::map<string, font>	fonts			= std::map<string, font>();
string					fonts_filepath	= L"-1";

font::font(string name) : sprite_batch(fonts_filepath + name + ".png", TEXT_CHAR_LIMIT, font_flags) {
	load_metadata(fonts_filepath + name);
}

void font::load_metadata(string name) {
	string cereal_filename(name + ".cereal");
	if (cereal_filename.file_exists()) {
		UNCEREAL(cereal_filename, meta);
	}
	else {
		std::fstream ifs;
		string filename = name + ".json";
		ifs.open((const char*)filename);
		char glyph;
		string tag;
		char in;
		int depth = 0;
		bool open_quote = false;
		bool escape = false;

		while (ifs >> in) {

			if (escape) { 
				escape = false;
				if (open_quote) { tag += in; }
				continue;
			}

			switch (in) {
			case '{':
				depth++;
				break;
			case '}':
				depth--;
				break;
			case '\\':
				escape = true;
				break;
			case '"':
				if (open_quote) {
					ifs >> in;
					assert(in == ':');
					switch (depth) {
					case 1:
						if		(tag == "name"		) { do { ifs >> in; } while (in != ','); }
						else if (tag == "size"		) { ifs >> meta.font_size; }
						else if (tag == "bold"		) { do { ifs >> in; } while (in != ','); }
						else if (tag == "italic"	) { do { ifs >> in; } while (in != ','); }
						else if (tag == "width"		) { do { ifs >> in; } while (in != ','); }
						else if (tag == "height"	) { do { ifs >> in; } while (in != ','); }
						else if (tag == "characters") { /* do nothing */ }
						break;
					case 2:
						if (tag.length() == 0) glyph = ' ';
						else glyph = tag[0];
						meta.atlas.try_emplace(glyph);
						break;
					case 3:
						if		(tag == "x"			) { ifs >> meta.atlas[glyph].pos.x;		}
						else if (tag == "y"			) { ifs >> meta.atlas[glyph].pos.y;		}
						else if (tag == "width"		) { ifs >> meta.atlas[glyph].size.x;	}
						else if (tag == "height"	) { ifs >> meta.atlas[glyph].size.y;	}
						else if (tag == "originX"	) { ifs >> meta.atlas[glyph].offset.x;	}
						else if (tag == "originY"	) { ifs >> meta.atlas[glyph].offset.y;	}
						else if (tag == "advance"	) { ifs >> meta.atlas[glyph].advance;	}
					}
				}
				else { tag = ""; }
				open_quote = !open_quote;
				break;
			default:
				if (open_quote) { tag += in; }
			}
		}

		assert(depth == 0);
		assert(open_quote == false);
		
		CEREAL(cereal_filename, meta);
	}
}

void font::render_all(float2 pos, float2 scale, float2 pivot, float rotation) {
	render_settings rs{ sampler::POINT, pixel_shader(DEFAULT_FLAT), vertex_shader(VARIABLE_VS), geometry_shader(VARIABLE_GS) };
	rs.set();
	float2 pen = pos;
	for (const char c : buffer) {
		auto it = meta.atlas.find(c);
		if (it == meta.atlas.end()) { continue; }
		const metadata::glyph& glyph = it->second;
		prerender(pen + float2{0, glyph.offset.y} * scale, scale, pivot, rotation, glyph.pos, glyph.size);
		pen.x += glyph.advance * scale.x;
	}
}

float font::get_width(string s) {
	float w = 0;
	for (const char c : s) {
		auto it = meta.atlas.find(c);
		if (it != meta.atlas.end()) { w += it->second.advance; }
	}
	return w;
}

font* get_font(string font_name) {
	font* out_font = nullptr;
	auto it = fonts.find(font_name);
	if (it != fonts.end()) { out_font = &it->second; }
	else {
		font newfont(font_name);
		out_font = &fonts.try_emplace(font_name, std::move(newfont)).first->second;
	}
	return out_font;
}

float text::width(string s, string font_name) {
	return get_font(font_name)->get_width(s);
}

float text::height(string font_name) {
	return get_font(font_name)->get_font_size();
}

float text::out(string s, float2 pos, float2 scale, string font_name, color color, float2 align) {
	
	font* out_font = get_font(font_name);

	out_font->set_buffer(s);
	pos -= float2{out_font->get_buffer_width(), out_font->get_font_size()} * scale * (align + float2(1, 1)) * 0.5f;

	out_font->render(pos, scale, C_BL, 0, color, float2{ 0 }, float2{ out_font->get_font_size() });

	return out_font->get_font_size() * scale.y;
}

void text::set_filepath(string path) { fonts_filepath = path; }

void text::uninit() { fonts.clear(); }