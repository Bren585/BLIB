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

			//window_name = name;
			//
			//WNDCLASSEXW wcex{};
			//wcex.cbSize = sizeof(WNDCLASSEX);
			//wcex.style = CS_HREDRAW | CS_VREDRAW;
			//wcex.lpfnWndProc = window_procedure;
			//wcex.cbClsExtra = 0;
			//wcex.cbWndExtra = 0;
			//wcex.hInstance = instance;
			//wcex.hIcon = 0;
			//wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			//wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			//wcex.lpszMenuName = NULL;
			//wcex.lpszClassName = window_name;
			//wcex.hIconSm = 0;
			//RegisterClassExW(&wcex);
			//
			//RECT rc{ 0, 0, SCREEN_W, SCREEN_H };
			//AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
			//hwnd = CreateWindowExW(0, window_name, L"", WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, instance, NULL);
			//ShowWindow(hwnd, cmd_show);
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

			//device::get()->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Device") - 1, "Device");
			//swap_chain->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("SwapChain") - 1, "SwapChain");
			//
			//ID3D11Texture2D* back_buffer{};														// render target view
			//{
			//	hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&back_buffer)); VERIFY;
			//	hr = device::get()->CreateRenderTargetView(back_buffer, NULL, render_target_view.GetAddressOf()); VERIFY;
			//}
			//back_buffer->Release();
			//
			//ID3D11Texture2D* depth_stencil_buffer{};
			//D3D11_TEXTURE2D_DESC texture2d_desc{};												// depth_stencil_buffer
			//{
			//	texture2d_desc.Width = SCREEN_W;
			//	texture2d_desc.Height = SCREEN_H;
			//	texture2d_desc.MipLevels = 1;
			//	texture2d_desc.ArraySize = 1;
			//	texture2d_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			//	texture2d_desc.SampleDesc.Count = 1;
			//	texture2d_desc.SampleDesc.Quality = 0;
			//	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
			//	texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			//	texture2d_desc.CPUAccessFlags = 0;
			//	texture2d_desc.MiscFlags = 0;
			//	hr = device::get()->CreateTexture2D(&texture2d_desc, NULL, &depth_stencil_buffer); VERIFY;
			//}
			//
			//D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};							// depth stencil view
			//{
			//	depth_stencil_view_desc.Format = texture2d_desc.Format;
			//	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			//	depth_stencil_view_desc.Texture2D.MipSlice = 0;
			//	hr = device::get()->CreateDepthStencilView(depth_stencil_buffer, &depth_stencil_view_desc, depth_stencil_view.GetAddressOf()); VERIFY;
			//}
			//depth_stencil_buffer->Release();
			//

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

			//D3D11_SAMPLER_DESC sampler_desc;													// sampler state
			//{
			//	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			//	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			//	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			//	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			//	sampler_desc.MipLODBias = 0;
			//	sampler_desc.MaxAnisotropy = 16;
			//	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			//	sampler_desc.BorderColor[0] = 0;
			//	sampler_desc.BorderColor[1] = 0;
			//	sampler_desc.BorderColor[2] = 0;
			//	sampler_desc.BorderColor[3] = 0;
			//	sampler_desc.MinLOD = 0;
			//	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
			//	hr = device::get()->CreateSamplerState(&sampler_desc, sampler_states[0].GetAddressOf()); VERIFY;
			//
			//	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			//	hr = device::get()->CreateSamplerState(&sampler_desc, sampler_states[1].GetAddressOf()); VERIFY;
			//
			//	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
			//	hr = device::get()->CreateSamplerState(&sampler_desc, sampler_states[2].GetAddressOf()); VERIFY;
			//}
			//
			//D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};										// depth stencil
			//{
			//	depth_stencil_desc.DepthEnable = TRUE;
			//	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			//	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			//	hr = device::get()->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[0].GetAddressOf());
			//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//
			//	depth_stencil_desc.DepthEnable = FALSE;
			//	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			//	hr = device::get()->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[1].GetAddressOf());
			//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//
			//	depth_stencil_desc.DepthEnable = TRUE;
			//	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			//	hr = device::get()->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[2].GetAddressOf());
			//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//
			//	depth_stencil_desc.DepthEnable = FALSE;
			//	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			//	hr = device::get()->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states[3].GetAddressOf());
			//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//}
			//
			//D3D11_BLEND_DESC blend_desc{};														// blend state
			//{
			//	// No Blending
			//	{
			//		blend_desc.AlphaToCoverageEnable = FALSE;
			//		blend_desc.IndependentBlendEnable = FALSE;
			//		blend_desc.RenderTarget[0].BlendEnable = TRUE;
			//		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			//		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			//		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			//		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			//		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			//		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			//		blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			//		hr = device::get()->CreateBlendState(&blend_desc, blend_states[BLEND_NONE].GetAddressOf());
			//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//	} 
			//
			//	// Alpha
			//	{
			//		blend_desc.IndependentBlendEnable = TRUE;
			//		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			//		hr = device::get()->CreateBlendState(&blend_desc, blend_states[BLEND_ALPHA].GetAddressOf());
			//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//	}
			//
			//	// Add 
			//	{
			//		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			//		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			//		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			//		hr = device::get()->CreateBlendState(&blend_desc, blend_states[BLEND_ADD].GetAddressOf());
			//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//	}
			//
			//	// Subtract
			//	{
			//		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
			//		hr = device::get()->CreateBlendState(&blend_desc, blend_states[BLEND_SUBTRACT].GetAddressOf());
			//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//	}
			//
			//	// Replace 
			//	{
			//		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			//		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			//		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			//		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			//		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			//		hr = device::get()->CreateBlendState(&blend_desc, blend_states[BLEND_REPLACE].GetAddressOf());
			//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//	}
			//
			//	// Multiply
			//	{
			//		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
			//		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
			//		hr = device::get()->CreateBlendState(&blend_desc, blend_states[BLEND_MULTIPLY].GetAddressOf());
			//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//	}
			//
			//	// Lighten
			//	{
			//		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			//		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			//		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MAX;
			//		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			//		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			//		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
			//		hr = device::get()->CreateBlendState(&blend_desc, blend_states[BLEND_LIGHTEN].GetAddressOf());
			//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//	}
			//
			//	// Darken
			//	{
			//		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MIN;
			//		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MIN;
			//		hr = device::get()->CreateBlendState(&blend_desc, blend_states[BLEND_DARKEN].GetAddressOf());
			//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//	}
			//
			//	// Screen
			//	{
			//		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			//		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			//		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			//		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			//		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			//		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			//		hr = device::get()->CreateBlendState(&blend_desc, blend_states[BLEND_SCREEN].GetAddressOf());
			//		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//	}
			//}
			//
			//D3D11_BUFFER_DESC buffer_desc{};													// constant buffer
			//{
			//	buffer_desc.ByteWidth = sizeof(scene_constants);
			//	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			//	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			//	buffer_desc.CPUAccessFlags = 0;
			//	buffer_desc.MiscFlags = 0;
			//	buffer_desc.StructureByteStride = 0;
			//	hr = device::get()->CreateBuffer(&buffer_desc, nullptr, constant_buffers[0].GetAddressOf());
			//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//}
			//
			//D3D11_RASTERIZER_DESC rasterizer_desc{};											// rasterizer desc
			//{
			//	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
			//	rasterizer_desc.CullMode = D3D11_CULL_BACK;
			//	rasterizer_desc.FrontCounterClockwise = false;
			//	rasterizer_desc.DepthBias = 0;
			//	rasterizer_desc.DepthBiasClamp = 0;
			//	rasterizer_desc.SlopeScaledDepthBias = 0;
			//	rasterizer_desc.DepthClipEnable = true;
			//	rasterizer_desc.ScissorEnable = false;
			//	rasterizer_desc.MultisampleEnable = false;
			//	rasterizer_desc.AntialiasedLineEnable = false;
			//	hr = device::get()->CreateRasterizerState(&rasterizer_desc, rasterizer_states[0].GetAddressOf());
			//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//
			//	rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
			//	rasterizer_desc.CullMode = D3D11_CULL_BACK;
			//	rasterizer_desc.AntialiasedLineEnable = true;
			//	hr = device::get()->CreateRasterizerState(&rasterizer_desc, rasterizer_states[1].GetAddressOf());
			//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//
			//	rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
			//	rasterizer_desc.CullMode = D3D11_CULL_NONE;
			//	rasterizer_desc.AntialiasedLineEnable = true;
			//	hr = device::get()->CreateRasterizerState(&rasterizer_desc, rasterizer_states[2].GetAddressOf());
			//	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			//}
		}

		/* INIT */ {
			render_target::init(swap_chain.Get());
			input::init();
			viewer::init();
			audio::init();
			HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		}

		/* Default Settings */ {
			render_target::focus_main();
			sampler::set(sampler::DEFAULT, 0);
			sampler::set(sampler::LINEAR, 1);
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
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

		UINT sync_interval{ 0 };
		HRESULT hr = swap_chain->Present(sync_interval, 0); VERIFY;
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

		viewer::uninit();
		audio::uninit();

		active = false;
		
		return 0;
	}

	float dtime() { return delta_time; }

	bool is_active() { return active; }

	void calculate_frame_stats()
	{
		if (++frames, (tictoc.time_stamp() - seconds) >= 1.0f)
		{
			float fps = static_cast<float>(frames);
			std::wostringstream outs;
			outs.precision(6);
			outs << window::name << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
			window::rename(outs.str().c_str());

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