#include "pch.h"
#include "input.h"

#include <Keyboard.h>
#include <Mouse.h>
#include "window.h"

using DirectX::Keyboard;
using DirectX::Mouse;
using std::map;

namespace BLIB {

	namespace input {

		map<Keyboard::Keys, keymask>	keybinds;
		std::unique_ptr<Keyboard>		keyboard			= nullptr;
		std::unique_ptr<Mouse>			mouse				= nullptr;
		keymask							_state				= 0;
		keymask							_trigger			= 0;
		keymask							_release			= 0;
		bool							mouse_lock			= false;
		float2							mouse_pos;
		float							mouse_scroll		= 0.0f;
		float							mouse_scroll_value	= 0.0f;
		bool							reset_scroll_value	= false;

		void init() {
			keyboard = std::make_unique<Keyboard>();
			mouse = std::make_unique<Mouse>();
			mouse->SetWindow(window::get());

#define AUTO_ASSIGN(x)		keybinds[Keyboard::x]			= key::x
#define AUTO_ASSIGN_LR(x)	keybinds[Keyboard::Left##x]		= key::L##x; \
							keybinds[Keyboard::Right##x]	= key::R##x;
#define ASSIGN(x, y)		keybinds[Keyboard::x]			= key::y

			AUTO_ASSIGN(Up);
			AUTO_ASSIGN(Down);
			AUTO_ASSIGN(Left);
			AUTO_ASSIGN(Right);
			AUTO_ASSIGN(Space);
			AUTO_ASSIGN(Enter);
			AUTO_ASSIGN(Back);
			AUTO_ASSIGN(X);
			AUTO_ASSIGN(Z);
			AUTO_ASSIGN(I);
			AUTO_ASSIGN(J);
			AUTO_ASSIGN(K);
			AUTO_ASSIGN(L);

			AUTO_ASSIGN_LR(Alt);
			AUTO_ASSIGN_LR(Shift);
			AUTO_ASSIGN_LR(Control);

			ASSIGN(W, Up);
			ASSIGN(S, Down);
			ASSIGN(A, Left);
			ASSIGN(D, Right);
			ASSIGN(Escape, Esc);
			ASSIGN(Back, Backspace);

#undef AUTO_ASSIGN
#undef AUTO_ASSIGN_LR
#undef ASSIGN
		}

		void update() {
			Keyboard::State key_state = keyboard->GetState();
			Mouse::State mouse_state = mouse->GetState();
			
			mouse_pos = { static_cast<float>(mouse_state.x), static_cast<float>(mouse_state.y) };
			if (mouse_state.positionMode == Mouse::MODE_ABSOLUTE) { mouse_pos = window::screen_to_local(mouse_pos); }

			mouse_scroll = mouse_state.scrollWheelValue - mouse_scroll_value;
			if (reset_scroll_value) {
				mouse->ResetScrollWheelValue();
				mouse_scroll_value = 0;
				reset_scroll_value = false;
			}
			else {
				mouse_scroll_value = static_cast<float>(mouse_state.scrollWheelValue);
			}

			keymask old = _state;
			_state = 0;

			for (auto& pair : keybinds) {
				if (key_state.IsKeyDown(pair.first))
				{
					_state |= pair.second;
				}
			}

			if (mouse_state.leftButton		) { _state |= key::LClick; }
			if (mouse_state.rightButton		) { _state |= key::RClick; }
			if (mouse_state.middleButton	) { _state |= key::MClick; }

			_trigger = ~old & _state;
			_release = old & ~_state;
		}

		bool mouse_locked()	{ return mouse_lock; }
		void set_mouse_locked(bool locked) { 
			if (mouse_lock == locked) return;
			mouse_lock = locked; 
			if (mouse_lock) { mouse->SetMode(Mouse::MODE_RELATIVE); }
			else			{ mouse->SetMode(Mouse::MODE_ABSOLUTE); }
		}

		float2 get_mouse_pos() { return mouse_pos; }
		float get_mouse_scroll() { return mouse_scroll; }
		float get_mouse_scroll_value() { return mouse_scroll_value; }
		void reset_mouse_scroll_value() { reset_scroll_value = true; }

		keymask state()   { return _state; }
		keymask trigger() { return _trigger; }
		keymask release() { return _release; }
	}
}