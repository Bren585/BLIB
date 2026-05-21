#include "pch.h"
#include "rasterize.h"

using std::map;
using Microsoft::WRL::ComPtr;

namespace BLIB {

	namespace rasterize {
		map<state, ComPtr<ID3D11RasterizerState>> rasterizer_states = map<state, ComPtr<ID3D11RasterizerState>>();
		state current_state = UNDEFINED;

		ID3D11RasterizerState* get(state state) {
			auto it = rasterizer_states.find(state);
			if (it != rasterizer_states.end()) { return it->second.Get(); }

			HRESULT hr{ S_OK };
			D3D11_RASTERIZER_DESC rast_desc{};

			//rast_desc defaults
			rast_desc.DepthBias = 0;
			rast_desc.DepthBiasClamp = 0;
			rast_desc.SlopeScaledDepthBias = 0;
			rast_desc.DepthClipEnable = true;
			rast_desc.ScissorEnable = false;
			rast_desc.MultisampleEnable = false;

			switch (state) {
			case FILL:
				rast_desc.FillMode = D3D11_FILL_SOLID;
				rast_desc.CullMode = D3D11_CULL_BACK;
				rast_desc.FrontCounterClockwise = false;
				rast_desc.AntialiasedLineEnable = false;
				break;
			case FILL_FLIP:
				rast_desc.FillMode = D3D11_FILL_SOLID;
				rast_desc.CullMode = D3D11_CULL_BACK;
				rast_desc.FrontCounterClockwise = true;
				rast_desc.AntialiasedLineEnable = false;
				break;
			case WIRE:
				rast_desc.FillMode = D3D11_FILL_WIREFRAME;
				rast_desc.CullMode = D3D11_CULL_NONE;
				rast_desc.FrontCounterClockwise = false;
				rast_desc.AntialiasedLineEnable = true;
				break;
			case WIRE_FLIP:
				rast_desc.FillMode = D3D11_FILL_WIREFRAME;
				rast_desc.CullMode = D3D11_CULL_NONE;
				rast_desc.FrontCounterClockwise = false;
				rast_desc.AntialiasedLineEnable = true;
				break;
			case WIRE_FRONT:
				rast_desc.FillMode = D3D11_FILL_WIREFRAME;
				rast_desc.CullMode = D3D11_CULL_BACK;
				rast_desc.FrontCounterClockwise = false;
				rast_desc.AntialiasedLineEnable = true;
				break;
			case WIRE_FRONT_FLIP:
				rast_desc.FillMode = D3D11_FILL_WIREFRAME;
				rast_desc.CullMode = D3D11_CULL_BACK;
				rast_desc.FrontCounterClockwise = true;
				rast_desc.AntialiasedLineEnable = true;
				break;
			default:
				_ASSERT_EXPR(false, L"Invalid Rasterizer State");
			}

			rasterizer_states.insert(std::make_pair(state, ComPtr<ID3D11RasterizerState>()));
			auto jt = rasterizer_states.find(state);
			hr = device::get()->CreateRasterizerState(&rast_desc, jt->second.GetAddressOf()); VERIFY;

			return jt->second.Get();
		}

		void set(state state) {
			if (state == current_state) return;

			current_state = state;
			device::context()->RSSetState(get(state));
		}

		void release_all() { rasterizer_states.clear(); }

	}
}