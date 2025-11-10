#pragma once
#include "object.h"
#include "render_target.h"
#include "text.h"

namespace BLIB {
	class canvas : public flat::object {
	private:
		mutable render_target::view view;
		color background;

	public:
		canvas(float2 size);

		void clear() const;

		void resize(float2 size);
		float2 get_size() const { return view.get_size(); }

		void draw(renderable* r, render_settings rs = {}) const;

		float type(string s, float2 pos, float2 size, font_name font = FONT_DEFAULT, color color = { 1.0f, 1.0f, 1.0f, 1.0f }, float2 align = { -1, -1 }) const;

		void focus	()	const { view.focus();	}
		void unfocus()	const { view.unfocus();	}

		operator render_target::view*				()			{ return &view; }
		render_target::view*			get_view	()			{ return &view; }
		const render_target::view&		peek_view	() const	{ return view;	}

		void render_to_main() const {
			render_target::focus_main();
			flat::object::render();
		}

		void	set_background(color c)			{ background = c; }
		color	get_background()		const	{ return background; }	
		
		void snapshot_to_sprite(sprite* target) const;
		
	};
}