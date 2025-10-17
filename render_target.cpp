#include "pch.h"
#include "render_target.h"
#include "sprite.h"
#include "window.h"

namespace BLIB {
	namespace render_target {

		std::unique_ptr<view> main_view = nullptr;
		color background = {0, 0, 0};

		view::view(IDXGISwapChain* swap_chain) : size(window::size()) {
			get_back_buffer(swap_chain);
			shader_resource_view = nullptr;
			create_depth_stencil();
		}

		view::view(float2 size) : size(size) {
			D3D11_TEXTURE2D_DESC tex_desc = {};
			{
				tex_desc.Width = (UINT)size.x;
				tex_desc.Height = (UINT)size.y;
				tex_desc.MipLevels = 1;
				tex_desc.ArraySize = 1;
				tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				tex_desc.SampleDesc.Count = 1;
				tex_desc.Usage = D3D11_USAGE_DEFAULT;
				tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				tex_desc.CPUAccessFlags = 0;
				tex_desc.MiscFlags = 0;
			}

			create_back_buffer(tex_desc);
			create_depth_stencil();
		}

		void view::create_back_buffer(D3D11_TEXTURE2D_DESC desc) {
			HRESULT hr;
			ID3D11Texture2D* back_buffer{};
			hr = device::get()->CreateTexture2D(&desc, nullptr, &back_buffer); VERIFY;
			assert(back_buffer);
			hr = device::get()->CreateRenderTargetView(back_buffer, nullptr, render_target_view.GetAddressOf()); VERIFY;
			hr = device::get()->CreateShaderResourceView(back_buffer, nullptr, shader_resource_view.GetAddressOf()); VERIFY;
			back_buffer->Release();
		}

		void view::get_back_buffer(IDXGISwapChain* swap_chain) {
			HRESULT hr;
			ID3D11Texture2D* back_buffer{};
			{
				hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&back_buffer)); VERIFY;
				assert(back_buffer);
				hr = device::get()->CreateRenderTargetView(back_buffer, NULL, render_target_view.GetAddressOf()); VERIFY;
			}
			back_buffer->Release();
		}

		void view::create_depth_stencil() {
			HRESULT hr;
			D3D11_TEXTURE2D_DESC depth_desc = {};
			{
				depth_desc.Width = (UINT)size.x;
				depth_desc.Height = (UINT)size.y;
				depth_desc.MipLevels = 1;
				depth_desc.ArraySize = 1;
				depth_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				depth_desc.SampleDesc.Count = 1;
				depth_desc.SampleDesc.Quality = 0;
				depth_desc.Usage = D3D11_USAGE_DEFAULT;
				depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				depth_desc.CPUAccessFlags = 0;
				depth_desc.MiscFlags = 0;
			}
			ID3D11Texture2D* depth_tex{};
			hr = device::get()->CreateTexture2D(&depth_desc, nullptr, &depth_tex); VERIFY;
			assert(depth_tex);
			hr = device::get()->CreateDepthStencilView(depth_tex, nullptr, depth_stencil_view.GetAddressOf()); VERIFY;
			depth_tex->Release();
		}

		void view::clear(color bkg) {
			FLOAT color[]{ bkg.r, bkg.g, bkg.b, bkg.a };
			device::context()->ClearRenderTargetView(render_target_view.Get(), color);
			device::context()->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}

		void view::resize(float2 size, IDXGISwapChain* swap_chain) {
			this->size = size;

			if (swap_chain) { 
				render_target_view.Reset();
				shader_resource_view.Reset();
				get_back_buffer(swap_chain); }
			else {
				D3D11_TEXTURE2D_DESC tex_desc;
				ID3D11Texture2D* back_buffer; 
				{
					render_target_view->GetResource(reinterpret_cast<ID3D11Resource**>(&back_buffer));
					back_buffer->GetDesc(&tex_desc);
				}
				back_buffer->Release();
				tex_desc.Width = static_cast<UINT>(size.x);
				tex_desc.Height = static_cast<UINT>(size.y);

				//D3D11_TEXTURE2D_DESC tex_desc = {};
				//{
				//	tex_desc.Width = (UINT)size.x;
				//	tex_desc.Height = (UINT)size.y;
				//	tex_desc.MipLevels = 1;
				//	tex_desc.ArraySize = 1;
				//	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				//	tex_desc.SampleDesc.Count = 1;
				//	tex_desc.Usage = D3D11_USAGE_DEFAULT;
				//	tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				//	tex_desc.CPUAccessFlags = 0;
				//	tex_desc.MiscFlags = 0;
				//}

				render_target_view.Reset();
				shader_resource_view.Reset();

				create_back_buffer(tex_desc);
			}

			depth_stencil_view.Reset();
			create_depth_stencil();
		}

		void view::focus() {
			unbind();
			device::context()->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());

			D3D11_VIEWPORT viewport{};
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = size.x;
			viewport.Height = size.y;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			device::context()->RSSetViewports(1, &viewport);

			sprite::set_viewport(size);
		}

		void view::release() {
			render_target_view.Reset();
			depth_stencil_view.Reset();
			shader_resource_view.Reset();
		}

		void init(IDXGISwapChain* swap_chain) { if (!main_view) main_view = std::make_unique<view>(swap_chain); }

		void clear_main		()											{ RENDER_LOCK;	main_view->clear(background);				}
		void resize_main	(float2 size, IDXGISwapChain* swap_chain)	{				main_view->resize(size, swap_chain);		}
		void focus_main		()											{				main_view->focus();							}
		void release_main	()											{				main_view->release();						}

		void unbind() { 
			ID3D11RenderTargetView* nullRTV[1] = { nullptr };
			device::context()->OMSetRenderTargets(1, nullRTV, nullptr);
			
			ID3D11ShaderResourceView* nullSRV[1] = { nullptr }; 
			device::context()->PSSetShaderResources(0, 1, nullSRV); 
		}

		void set_main_background(color c) { background = c; }

	}
};