#pragma once
#include "sprite_batch.h"
#include "math.h"
#include "string.h"
#include "cereal.h"

#define TEXT_CHAR_LIMIT		256
#define FONT_DEFAULT		"Arial"

//enum font_name {
//	FONT_0 = 0,
//	FONT_1,
//	FONT_2,
//	FONT_3,
//	BLOCK,
//	DEBUG_FONT,
//
//	COUNT_FONT,
//	FONT_DEFAULT = FONT_0,
//};

namespace BLIB {

	namespace text {

		//string get_filename_from_font(font_name f)
		float	out(string s, float2 pos, float2 scale, string font_name = FONT_DEFAULT, color color = { 1.0f, 1.0f, 1.0f, 1.0f }, float2 align = {-1, -1});
		void	set_filepath(string path);
		float	width(string s, string font_name = FONT_DEFAULT);
		float	height(string font_name = FONT_DEFAULT);

		class font : public sprite_batch {
		private:
			struct metadata {
				struct glyph {
					float2 pos;
					float2 size;
					float2 offset;
					float advance;
					SERIALIZE(pos, size, offset, advance)
				};

				float font_size = 0;
				std::unordered_map<char, glyph> atlas{};
				SERIALIZE(font_size, atlas)
			};

			metadata meta{};
			string buffer;

			void render_all(float2 pos, float2 scale, float2 pivot, float rotation) override;
			void load_metadata(string name);
		public:
			font(string name);

			void set_buffer(string b) { buffer = b; }
			float get_font_size() { return meta.font_size; }
			float get_width(string s);
			float get_buffer_width() { return get_width(buffer); }
		};

		void uninit();

	}
}