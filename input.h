#pragma once
#include "math.h"

typedef unsigned long long keymask;

namespace key {
#define KEY_START_LINE 9
#define KEY(name) constexpr keymask name = (1ULL << (__LINE__ - KEY_START_LINE))

	KEY(Up);
	KEY(Down);
	KEY(Left);
	KEY(Right);
	KEY(Enter);
	KEY(Back);
	KEY(Space);
	KEY(Z);
	KEY(X);
	KEY(I);
	KEY(J);
	KEY(K);
	KEY(L);
	KEY(Esc);
	KEY(LClick);
	KEY(RClick);
	KEY(MClick);
	KEY(LShift);
	KEY(RShift);
	KEY(LAlt);
	KEY(RAlt);
	KEY(LControl);
	KEY(RControl);
	KEY(Backspace);

	constexpr keymask Shift		= LShift	| RShift;
	constexpr keymask Alt		= LAlt		| RAlt;
	constexpr keymask Control	= LControl	| RControl;

#undef KEY
}

namespace BLIB {
	
	namespace input {

		void init();
		void update();
		void wm_char(wchar_t c);

		bool mouse_locked();
		void set_mouse_locked(bool locked);

		float2 get_mouse_pos();
		float get_mouse_scroll();
		float get_mouse_scroll_value();
		void reset_mouse_scroll_value();

		keymask state();
		keymask trigger();
		keymask release();

		inline bool state	(keymask key) {return key & state	();}
		inline bool trigger	(keymask key) {return key & trigger	();}
		inline bool release	(keymask key) {return key & release	();}

		string& get_typing_buffer();

	}

}
