#include "pch.h"
#include "render_target.h"
#include "sprite.h"
#include "window.h"
#include "texture.h"

namespace BLIB {

	string screenshot_filepath = "-1";
	string queued_screenshot = "";

	void set_screenshot_filepath(string filepath) { screenshot_filepath = filepath; }

	namespace render_target {

		std::unique_ptr<view> main_view = nullptr;
		color background = {0, 0, 0};

		view* active_views[MAX_VIEWS]{ nullptr };
		view* depth_view = nullptr;

		void bind_all() {
			unbind();

			ID3D11RenderTargetView* rtvs[MAX_VIEWS] = {nullptr};
			ID3D11DepthStencilView* dsv{nullptr};
			float2 ds_size;

			if (depth_view) {
				dsv = depth_view->depth_stencil_view.Get();
				ds_size = depth_view->get_size();
			}

			for (int i = 0; i < MAX_VIEWS; i++) {
				view*& rt = active_views[i];
				if (rt) {
					if (!dsv && rt->depth_stencil_view) {
						dsv = rt->depth_stencil_view.Get();
						ds_size = rt->get_size();
					}
					//if (rt->get_size() == ds_size) { rtvs[i] = rt->render_target_view.Get(); }
					//else { assert(false); }
					rtvs[i] = rt->render_target_view.Get();
				}
			}

			device::context()->OMSetRenderTargets(MAX_VIEWS, rtvs, dsv);
		}

		view::view(IDXGISwapChain* swap_chain) : size(window::size()), swap_chain_ptr(swap_chain) {
			get_back_buffer();
			shader_resource_view = nullptr;
			create_depth_stencil();
		}

		view::view(float2 size, DXGI_FORMAT format) : size(size), swap_chain_ptr(nullptr) {
			D3D11_TEXTURE2D_DESC tex_desc = {};
			{
				tex_desc.Width				= (UINT)size.x;
				tex_desc.Height				= (UINT)size.y;
				tex_desc.MipLevels			= 1;
				tex_desc.ArraySize			= 1;
				tex_desc.Format				= format;
				tex_desc.SampleDesc.Count	= 1;
				tex_desc.Usage				= D3D11_USAGE_DEFAULT;
				tex_desc.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				tex_desc.CPUAccessFlags		= 0;
				tex_desc.MiscFlags			= 0;
			}

			create_back_buffer(tex_desc);
			create_depth_stencil();
		}

		void view::create_back_buffer(D3D11_TEXTURE2D_DESC desc) {
			HRESULT hr;
			ID3D11Texture2D* back_buffer{};
			hr = device::get()->CreateTexture2D(&desc, nullptr, &back_buffer); VERIFY;
			assert(back_buffer);
			hr = device::get()->CreateRenderTargetView(back_buffer, nullptr, render_target_view.ReleaseAndGetAddressOf()); VERIFY;
			hr = device::get()->CreateShaderResourceView(back_buffer, nullptr, shader_resource_view.ReleaseAndGetAddressOf()); VERIFY;
			back_buffer->Release();
		}

		void view::get_back_buffer() {
			assert(swap_chain_ptr);
			HRESULT hr;
			ID3D11Texture2D* back_buffer{};
			{
				hr = swap_chain_ptr->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&back_buffer)); VERIFY;
				assert(back_buffer);
				hr = device::get()->CreateRenderTargetView(back_buffer, NULL, render_target_view.ReleaseAndGetAddressOf()); VERIFY;
			}
			back_buffer->Release();
		}

		void view::create_depth_stencil() {
			HRESULT hr;
			ID3D11Texture2D* depth_tex{}; {
				D3D11_TEXTURE2D_DESC desc = {};
				desc.Width				= (UINT)size.x;
				desc.Height				= (UINT)size.y;
				desc.MipLevels			= 1;
				desc.ArraySize			= 1;
				desc.Format				= DXGI_FORMAT_R24G8_TYPELESS;
				desc.SampleDesc.Count	= 1;
				desc.SampleDesc.Quality = 0;
				desc.Usage				= D3D11_USAGE_DEFAULT;
				desc.BindFlags			= D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags		= 0;
				desc.MiscFlags			= 0;

				hr = device::get()->CreateTexture2D(&desc, nullptr, &depth_tex); VERIFY;
			}
			assert(depth_tex);
			
			/* Stencil */ {
				D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
				desc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
				desc.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = 0;

				hr = device::get()->CreateDepthStencilView(depth_tex, &desc, depth_stencil_view.ReleaseAndGetAddressOf()); VERIFY;
			}
			/* Depth SRV */ {
				D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
				desc.Format						= DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				desc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipLevels		= 1;
				desc.Texture2D.MostDetailedMip	= 0;

				hr = device::get()->CreateShaderResourceView(depth_tex, &desc, depth_shader_resource.ReleaseAndGetAddressOf()); VERIFY;
			}

			depth_tex->Release();
		}

		void view::clear(color bkg) {
			FLOAT color[]{ bkg.r, bkg.g, bkg.b, bkg.a };
			device::context()->ClearRenderTargetView(render_target_view.Get(), color);
			device::context()->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}

		void view::resize(float2 size) {
			this->size = size;

			if (swap_chain_ptr) { // window only, ergo not applicable to scenes
				swap_chain_ptr->ResizeBuffers(0, static_cast<uint>(size.x), static_cast<uint>(size.y), DXGI_FORMAT_UNKNOWN, 0);
				render_target_view.Reset();
				shader_resource_view.Reset();
				get_back_buffer(); 
			}
			else {
				D3D11_TEXTURE2D_DESC tex_desc; {
					Microsoft::WRL::ComPtr<ID3D11Resource> resource;
					Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
					render_target_view->GetResource(&resource);
					resource.As(&back_buffer);
					back_buffer->GetDesc(&tex_desc);
				}
				tex_desc.Width = static_cast<UINT>(size.x);
				tex_desc.Height = static_cast<UINT>(size.y);
				render_target_view.Reset();
				shader_resource_view.Reset();
				create_back_buffer(tex_desc);
			}

			depth_stencil_view.Reset();
			create_depth_stencil();
		}

		void view::save_to_file(string filename) {
			_ASSERT_EXPR_A(screenshot_filepath != "-1", "Must Set Filepath");

			string full_filepath	= screenshot_filepath + filename;

			if ((full_filepath + SCREENSHOT_EXT).file_exists()) {
				int i = 0;
				string numbered_filename;
				do {
					numbered_filename = full_filepath + string('(', i, ')') + SCREENSHOT_EXT;
				} while (numbered_filename.file_exists());
				full_filepath = numbered_filename;
			}
			else {
				full_filepath += SCREENSHOT_EXT;
			}

			if (swap_chain_ptr) { // swap chain branch
				RENDER_LOCK;
				device::context()->Flush();

				HRESULT hr{ S_OK }; 
				Microsoft::WRL::ComPtr<ID3D11Texture2D> back_resource, staged_resource; 
				hr = swap_chain_ptr->GetBuffer(0, IID_PPV_ARGS(&back_resource)); VERIFY;

				D3D11_TEXTURE2D_DESC desc = {};
				back_resource->GetDesc(&desc);
				desc.Usage			= D3D11_USAGE_STAGING;
				desc.BindFlags		= 0;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				desc.MiscFlags		= 0;

				hr = device::get()->CreateTexture2D(&desc, nullptr, staged_resource.GetAddressOf()); VERIFY;
				device::context()->CopyResource(staged_resource.Get(), back_resource.Get());

				texture::save_to_file(staged_resource.Get(), full_filepath);
			}
			else {
				texture::save_to_file(shader_resource_view.Get(), full_filepath);
			}
		}

		bool view::focus(int slot) {
			assert(slot < MAX_VIEWS && slot > FOCUS_NONE);
			bool was_free = true;
			if (active_in_slot != FOCUS_NONE) { unfocus(); was_free = false; }

			switch (slot) {
			case FOCUS_OVERWRITE:
				for (int i = 0; i < MAX_VIEWS; i++) { cached_views[i] = active_views[i]; active_views[i] = nullptr; }
				active_views[0] = this;
				cached_depth	= depth_view;
				depth_view		= nullptr;
				break;
			case FOCUS_DEPTH:
				for (int i = 0; i < MAX_VIEWS; i++) { cached_views[i] = active_views[i]; active_views[i] = nullptr; }
				cached_depth	= depth_view;
				depth_view		= this;
				break;
			default:
				cached_views[slot] = active_views[slot];
				active_views[slot] = this;
			}
			bind_all();
			active_in_slot = slot;

			cached_viewport = sprite::get_viewport();

			D3D11_VIEWPORT viewport{};
			viewport.TopLeftX	= 0;
			viewport.TopLeftY	= 0;
			viewport.Width		= size.x;
			viewport.Height		= size.y;
			viewport.MinDepth	= 0.0f;
			viewport.MaxDepth	= 1.0f;
			device::context()->RSSetViewports(1, &viewport);

			sprite::set_viewport(size);
			return was_free;
		}

		void view::unfocus() {
			switch (active_in_slot) {
			case FOCUS_NONE:
				return;
			case FOCUS_OVERWRITE:
			case FOCUS_DEPTH:
				for (int i = 0; i < MAX_VIEWS; i++) { active_views[i] = cached_views[i]; cached_views[i] = nullptr; }
				depth_view = cached_depth;
				cached_depth = nullptr;
				break;
			default:
				active_views[active_in_slot] = cached_views[active_in_slot];
				cached_views[active_in_slot] = nullptr;
			}
			bind_all();
			active_in_slot = FOCUS_NONE;

			D3D11_VIEWPORT viewport{};
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = cached_viewport.x;
			viewport.Height = cached_viewport.y;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			device::context()->RSSetViewports(1, &viewport);

			sprite::set_viewport(cached_viewport);
		}

		void view::release() {
			render_target_view.Reset();
			depth_stencil_view.Reset();
			shader_resource_view.Reset();
		}

		void init(IDXGISwapChain* swap_chain) { if (!main_view) main_view = std::make_unique<view>(swap_chain); }

		void clear_main				()				{ RENDER_LOCK;	main_view->clear(background);	}
		void resize_main			(float2 size)	{				main_view->resize(size);		}
		void focus_main				()				{				main_view->focus();				}
		void unfocus_main			()				{				main_view->unfocus();			}
		void release_main			()				{				main_view->release();			}
		void set_main_background	(color c)		{				background = c;					}

		view* _private::get_main() { return main_view.get(); }

		void unbind() { 
			ID3D11RenderTargetView* nullRTV[MAX_VIEWS] = { nullptr };
			device::context()->OMSetRenderTargets(MAX_VIEWS, nullRTV, nullptr);
			
			ID3D11ShaderResourceView* nullSRV[10] = { nullptr }; 
			device::context()->PSSetShaderResources(0, 10, nullSRV); 
		}

		void unfocus_all() {
			for (int i = 0; i < MAX_VIEWS; i++) active_views[i] = nullptr;
			depth_view = nullptr;
		}

	}

	void screenshot(string filename) { queued_screenshot = filename; }

	string empty_screenshot_queue() { string out = queued_screenshot; queued_screenshot = ""; return out; }
}