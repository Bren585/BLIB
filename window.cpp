#include "pch.h"
#include "window.h"
#include "manager.h"

namespace BLIB {
	namespace window {

		HWND		hwnd;
		window_mode _mode = DEFAULT_WINDOW_MODE;
		float2		_size = { DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT };
		string		_name;

		Microsoft::WRL::ComPtr<IDXGISwapChain>* swap_chain = nullptr;

		void create(HINSTANCE instance, int cmd_show, WNDPROC window_procedure, const wchar_t* name, Microsoft::WRL::ComPtr<IDXGISwapChain>* swap_chain_ptr) {
			_name = name;
			swap_chain = swap_chain_ptr;

			WNDCLASSEXW wcex{};
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = window_procedure;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = instance;
			wcex.hIcon = 0;
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = NULL;
			wcex.lpszClassName = _name;
			wcex.hIconSm = 0;
			RegisterClassExW(&wcex);

			RECT rc{ 0, 0, static_cast<LONG>(_size.x), static_cast<LONG>(_size.y) };

			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
			hwnd = CreateWindowExW(0, _name, L"", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, instance, NULL);
			ShowWindow(hwnd, cmd_show);
		}

		HWND get() { return hwnd; }

		void resize(float2 size) {
			_size = size;

			render_target::unbind();
			render_target::release_main();
			swap_chain->Get()->ResizeBuffers(0, static_cast<UINT>(size.x), static_cast<UINT>(size.y), DXGI_FORMAT_UNKNOWN, 0);
			render_target::resize_main(size, swap_chain->Get());

			manager::resize(size);
		}

		void rename(const wchar_t* name) { SetWindowTextW(hwnd, name); }

		const wchar_t* name() { return _name; }

		void set_mode(window_mode new_mode) {
			if (new_mode == _mode) { return; }

			if		(new_mode	== fullscreen) { swap_chain->Get()->SetFullscreenState(TRUE, nullptr); }
			else if (_mode		== fullscreen) { swap_chain->Get()->SetFullscreenState(FALSE, nullptr); }

			_mode = new_mode;
		}

		window_mode mode()			{ return _mode; }
		bool		is_fullscreen() { return _mode == fullscreen; }

		float2 size() { return _size; }
		float2 screen_to_local(float2 screen_pos) { POINT pt{ (int)screen_pos.x, (int)screen_pos.y }; ScreenToClient(hwnd, &pt); return { (float)pt.x, (float)pt.y }; }
	}
}