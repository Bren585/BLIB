#pragma once

#include <wrl.h>

struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;

struct HWND__;
typedef HWND__* HWND;

typedef unsigned int UINT;
#if defined(_WIN64)
typedef unsigned __int64 UINT_PTR;
typedef __int64          LONG_PTR;
#else
typedef unsigned int     UINT_PTR;
typedef long             LONG_PTR;
#endif
typedef LONG_PTR LRESULT;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LRESULT(__stdcall* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

struct IDXGISwapChain;
class float2;

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720
#define DEFAULT_WINDOW_MODE BLIB::window::windowed

namespace BLIB {
	namespace window {

		enum window_mode {
			windowed,
			minimized,
			fullscreen
		};

		void create(HINSTANCE instance, int cmd_show, WNDPROC window_procedure, const wchar_t* name, IDXGISwapChain* swap_chain_ptr);
		HWND get();

		void set_size(float2 size);
		void internal_update(float2 size);
		void resolve_resize();

		void rename(const wchar_t* name);
		const wchar_t* name();

		void		set_mode(window_mode new_mode);
		window_mode mode();
		bool		is_fullscreen();

		float2 size();
		float2 screen_to_local(float2 screen_pos);

	}
}