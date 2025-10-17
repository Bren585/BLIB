#include "pch.h"
#include "depth_stencil.h"

using std::map;
using Microsoft::WRL::ComPtr;

namespace BLIB {

	namespace stencil {
		map<state, ComPtr<ID3D11DepthStencilState>> rasterizer_states = map<state, ComPtr<ID3D11DepthStencilState>>();
		state current_state = UNDEFINED;

		ID3D11DepthStencilState* get(state state) {
			auto it = rasterizer_states.find(state);
			if (it != rasterizer_states.end()) { return it->second.Get(); }

			HRESULT hr{ S_OK };
			D3D11_DEPTH_STENCIL_DESC ds_desc{};

			//rast_desc defaults
			ds_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

			switch (state) {
			case DEPTH_MASK:
				ds_desc.DepthEnable = TRUE;
				ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				break;
			case DEPTH_NONE:
				ds_desc.DepthEnable = TRUE;
				ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
				break;
			case SURFACE_MASK:
				ds_desc.DepthEnable = FALSE;
				ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				break;
			case SURFACE_NONE:
				ds_desc.DepthEnable = FALSE;
				ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
				break;
			default:
				_ASSERT_EXPR(false, L"Invalid Rasterizer State");
			}

			rasterizer_states.insert(std::make_pair(state, ComPtr<ID3D11DepthStencilState>()));
			auto jt = rasterizer_states.find(state);
			hr = device::get()->CreateDepthStencilState(&ds_desc, jt->second.GetAddressOf()); VERIFY;

			return jt->second.Get();
		}

		void set(state state) {
			if (state == current_state) return;

			current_state = state;
			device::context()->OMSetDepthStencilState(get(state), 1);
		}
	}
}