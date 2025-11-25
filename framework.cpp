#include "pch.h"
#include "BLIB.h"

#include <Keyboard.h>
#include <Mouse.h>
#include <tchar.h>
using Microsoft::WRL::ComPtr;

#include "texture.h"

#include "window.h"

#ifdef _DEBUG
#include "./imgui/imgui.h"
#include "./imgui/imgui_internal.h"
#include "./imgui/imgui_impl_dx11.h"
#include "./imgui/imgui_impl_win32.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
#endif

static LRESULT CALLBACK window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return BLIB::is_active() ? BLIB::handle_message(hwnd, msg, wparam, lparam) : DefWindowProc(hwnd, msg, wparam, lparam);
}

namespace BLIB {

	//HWND	hwnd = nullptr;
	MSG		msg{};
	bool	resizing	{ false };
	bool	active		{ false };

	std::recursive_mutex	render_lock;

	ComPtr<IDXGISwapChain>	swap_chain;

	high_resolution_timer	tictoc;
	uint32_t				frames{ 0 };
	float					seconds{ 0 };
	//string				window_name;
	int						target_fps{ 60 };
	float					target_frame_time{ 0.0f };
	float					delta_time{ 0.0f };

	bool init(_In_ HINSTANCE instance, _In_opt_  HINSTANCE prev_instance, _In_ LPSTR cmd_line, _In_ int cmd_show, const wchar_t* name)
	{
#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(####);
#endif

		/* Window Setup */ {
			window::create(instance, cmd_show, window_procedure, name, &swap_chain);
		}

		/* Create Com Objects */ {
			HRESULT hr{ S_OK };

			UINT create_device_flags{ 0 };

#ifdef _DEBUG
			create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			DXGI_SWAP_CHAIN_DESC swap_chain_desc{};												// device and swap chain
			{
				swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swap_chain_desc.BufferCount = 2;
				swap_chain_desc.BufferDesc.Width = static_cast<UINT>(window::size().x);
				swap_chain_desc.BufferDesc.Height = static_cast<UINT>(window::size().y);
				swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
				swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
				swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swap_chain_desc.OutputWindow = window::get();
				swap_chain_desc.SampleDesc.Count = 1;
				swap_chain_desc.SampleDesc.Quality = 0;
				swap_chain_desc.Windowed = DEFAULT_WINDOW_MODE == window::windowed;

				D3D_FEATURE_LEVEL feature_levels{ D3D_FEATURE_LEVEL_11_0 };
				hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, create_device_flags, &feature_levels, 1, D3D11_SDK_VERSION, &swap_chain_desc, swap_chain.GetAddressOf(), device::get_address(), NULL, device::context_address()); VERIFY;
			}

			device::init_annotation();

			D3D11_VIEWPORT viewport{};															// default viewport
			{
				viewport.TopLeftX = 0;
				viewport.TopLeftY = 0;
				viewport.Width = window::size().x;
				viewport.Height = window::size().y;
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				device::context()->RSSetViewports(1, &viewport);
			}

		}

		/* INIT */ {
			render_target::init(swap_chain.Get());
			input::init();
			lighting::init();
			audio::init();
			HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		}

		/* Default Settings */ {
			render_target::focus_main();
			sampler::set(sampler::DEFAULT, 0);
			sampler::set(sampler::LINEAR, 1);
			sampler::set(sampler::COMPARE, 2);
			sampler::set(sampler::CLAMP_POINT, 3);
			stencil::set();
			blend::set();
			rasterize::set();

			target_frame_time = 1.0f / target_fps;
		}

		/* ImGUI */ {
#ifdef _DEBUG
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f, nullptr, glyphRangesJapanese);
			ImGui_ImplWin32_Init(window::get());
			ImGui_ImplDX11_Init(device::get(), device::context());
			ImGui::StyleColorsDark();
#endif
		}

		active = true;
		return true;
	}

	bool loop()
	{
		delta_time = 0;
		while (delta_time < target_frame_time) {
			if (WM_QUIT != msg.message)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					tictoc.tick();
					delta_time += tictoc.time_interval();
				}
			}
			else { return false; }
		}
#ifdef _DEBUG
		calculate_frame_stats();
#endif

		update();
		render_target::clear_main();
		active = manager::tick(delta_time);
		manager::display();
		render();

		return active;
	}

	void update() {
		input::update();
		audio::update();
#ifdef _DEBUG
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
#endif
	}

	void render()
	{
		RENDER_LOCK;

#ifdef _DEBUG
		{
			annotate("ImGui");
			//if (ImGui::Begin("Breakpoint")) {
			//	if (ImGui::Button("Break")) {
			//		assert(false);
			//	}
			//}
			//ImGui::End();
			shader::set_gs(NULL_SHADER);
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
#endif

		UINT sync_interval{ 0 };
		HRESULT hr = swap_chain->Present(sync_interval, 0); VERIFY;

		ID3D11Buffer* empty_buffer[8] = { nullptr };
		device::context()->PSSetConstantBuffers(0, 8, empty_buffer);
		device::context()->VSSetConstantBuffers(0, 8, empty_buffer);
		device::context()->GSSetConstantBuffers(0, 8, empty_buffer);
	}

	int uninit()
	{
		// ERROR CODE : return static_cast<int>(msg.wParam);
		manager::kill();

#ifdef _DEBUG
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
#endif

		BOOL fullscreen = 0;
		swap_chain->GetFullscreenState(&fullscreen, 0);
		if (fullscreen)
		{
			swap_chain->SetFullscreenState(FALSE, 0);
		}

		texture::release_all();

		CoUninitialize();

		audio::uninit();

		active = false;

		device::context()->ClearState();
		device::context()->Flush();
		swap_chain.Reset();
		device::reset();
		
		return 0;
	}

	float dtime() { return delta_time; }

	bool is_active() { return active; }

	void calculate_frame_stats()
	{
		if (++frames, (tictoc.time_stamp() - seconds) >= 1.0f)
		{
#ifdef _DEBUG
			float fps = static_cast<float>(frames);
			std::wostringstream outs;
			outs.precision(6);
			outs << window::name() << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
			window::rename(outs.str().c_str());
#endif

			frames = 0;
			seconds += 1.0f;
		}
	}

	LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
#ifdef _DEBUG
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }
#endif
		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps{};
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_CREATE:
			break;
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:
			DirectX::Keyboard::ProcessMessage(msg, wparam, lparam);
			break;
		case WM_MOUSEACTIVATE:
			return MA_ACTIVATEANDEAT;
		case WM_ACTIVATE:
		case WM_INPUT:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEHOVER:
			DirectX::Mouse::ProcessMessage(msg, wparam, lparam);
			break;
		case WM_ACTIVATEAPP:
			DirectX::Keyboard::ProcessMessage(msg, wparam, lparam);
			DirectX::Mouse::ProcessMessage(msg, wparam, lparam);
			break;
		case WM_ENTERSIZEMOVE:
			resizing = true;
			tictoc.stop();
			break;
		case WM_EXITSIZEMOVE:
			resizing = false;
			RECT rc;
			GetClientRect(window::get(), &rc);
			window::resize({static_cast<float>(rc.right - rc.left), static_cast<float>(rc.bottom - rc.top)});
			tictoc.start();
			break;
		case WM_SIZE:
			switch (wparam) {
			case SIZE_MINIMIZED:
				window::set_mode(window::minimized);
				break;
			case SIZE_MAXIMIZED:
				window::set_mode(window::windowed);
				window::resize({ static_cast<float>(LOWORD(lparam)), static_cast<float>(HIWORD(lparam)) });
				break;
			case SIZE_RESTORED:
				if (!resizing) {
					window::set_mode(window::windowed);
					window::resize({ static_cast<float>(LOWORD(lparam)), static_cast<float>(HIWORD(lparam)) });
				}
				break;
			}
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

}