#include "pch.h"
#include "sampler.h"

using std::map;
using Microsoft::WRL::ComPtr;

namespace BLIB {

	namespace sampler {
		map<state, ComPtr<ID3D11SamplerState>> sampler_states;
		state current_state = UNDEFINED;

		ID3D11SamplerState*const* get(state state) {
			auto it = sampler_states.find(state);
			if (it != sampler_states.end()) { return it->second.GetAddressOf(); }

			HRESULT hr{ S_OK };
			D3D11_SAMPLER_DESC sampler_desc{};

			//sampler_desc defaults
			sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.MipLODBias = 0;
			sampler_desc.MaxAnisotropy = 16;
			sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			sampler_desc.BorderColor[0] = 0;
			sampler_desc.BorderColor[1] = 0;
			sampler_desc.BorderColor[2] = 0;
			sampler_desc.BorderColor[3] = 0;
			sampler_desc.MinLOD = 0;
			sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

			switch (state) {
			case POINT:
				sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				break;
			case CLAMP_POINT:
				sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				break;
			case LINEAR:
				sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				break;
			case ANISOTROPIC:
				sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
				break;
			case COMPARE:
				sampler_desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
				sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
				sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
				break;
			default:
				_ASSERT_EXPR(false, L"Invalid Sampler State");
			}

			sampler_states.insert(std::make_pair(state, ComPtr<ID3D11SamplerState>()));
			auto jt = sampler_states.find(state);
			hr = device::get()->CreateSamplerState(&sampler_desc, jt->second.GetAddressOf()); VERIFY;

			return jt->second.GetAddressOf();
		}

		void set(state state, int slot) {
			if (state == current_state) return;

			current_state = state;
			device::context()->PSSetSamplers(slot, 1, get(state));
		}
	}
}