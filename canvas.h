#pragma once
#include "object.h"
#include "render_target.h"
#include "text.h"

namespace BLIB {
	class canvas : public flat::object {
	private:
		render_target::view view;
		color background;

	public:
		canvas(float2 size);

		void clear();

		void resize(float2 size);
		float2 get_size() const { return view.get_size(); }

		void draw(renderable* r);

		float type(string s, float2 pos, float2 size, font_name font = FONT_DEFAULT, color color = { 1.0f, 1.0f, 1.0f, 1.0f }, float2 align = { -1, -1 });

		void focus() { view.focus(); }

		void render_to_main() {
			render_target::focus_main();
			flat::object::render();
		}

		void	set_background(color c)			{ background = c; }
		color	get_background()		const	{ return background; }

		const render_target::view& peek_view() const { return view; }
		
		void snapshot_to_sprite(sprite* target);
		
	};
}