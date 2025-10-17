#pragma once

#include <windows.h>
#include <wrl.h>
#include <d3d11.h>

#include "math.h"
#include "window.h"

namespace BLIB {
	
	namespace render_target {

		class view {
		private:
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		render_target_view;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		depth_stencil_view;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shader_resource_view;
			float2												size;

			void create_back_buffer(D3D11_TEXTURE2D_DESC desc);
			void get_back_buffer(IDXGISwapChain* swap_chain);
			void create_depth_stencil();

		public:
			view(float2 size = window::size());
			view(IDXGISwapChain* swap_chain);

			void clear(color bkg);
			void resize(float2 size, IDXGISwapChain* swap_chain = nullptr);
			void focus();
			void release();
			
			void copy_SRV_to(ID3D11ShaderResourceView** dest)	const { shader_resource_view.CopyTo(dest); }
			void get_SRV_resource(ID3D11Resource** out)			const { shader_resource_view->GetResource(out); }

			float2 get_size() const { return size; }
		};

		void init(IDXGISwapChain* swap_chain);
		void clear_main();
		void resize_main(float2 size, IDXGISwapChain* swap_chain);
		void focus_main();
		void release_main();

		void unbind();

		void set_main_background(color c);
	}
}