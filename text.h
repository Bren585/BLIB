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

	COUNT_FONT,
	FONT_DEFAULT = FONT_0,
};

namespace BLIB {

	namespace text {

		string  get_filename_from_font(font_name f);
		float	out(string s, float2 pos, float2 size, font_name font = FONT_DEFAULT, color color = { 1.0f, 1.0f, 1.0f, 1.0f }, float2 align = {-1, -1});
		void	set_filepath(string path);

		class font : public sprite_batch {
		public:
			font(font_name f) : sprite_batch(get_filename_from_font(f), TEXT_CHAR_LIMIT, font_flags) {}
			void write(string s, float2 pos, float2 size, color color = { 1, 1, 1 });
		};

	}
}