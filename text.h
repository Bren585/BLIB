#pragma once
#include "sprite_batch.h"
#include "math.h"
#include "string.h"

#define TEXT_CHAR_LIMIT		256

enum font_name {
	FONT_0 = 0,
	FONT_1,
	FONT_2,
	FONT_3,
	BLOCK,
	DEBUG_FONT,

	COUNT_FONT,
	FONT_DEFAULT = FONT_0,
};

namespace BLIB {

	namespace text {

		string  get_filename_from_font(font_name f);
		float	out(string s, float2 pos, float2 size, font_name font = FONT_DEFAULT, color color = { 1.0f, 1.0f, 1.0f, 1.0f }, float2 align = {-1, -1});
		void	set_filepath(string path);

		class font : public sprite_batch {
		private:
			void render_each(float2 pos, float2 scale, float2 pivot, float rotation, float2 tile_index, float2 tile_size) override;
		public:
			string buffer;
			font(font_name f) : sprite_batch(get_filename_from_font(f), TEXT_CHAR_LIMIT, font_flags) {}
		};

	}
}