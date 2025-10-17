#include "pch.h"
#include "blend.h"

using std::map;
using Microsoft::WRL::ComPtr;

namespace BLIB {

	namespace blend {
		map<state, ComPtr<ID3D11BlendState>> blend_states = map<state, ComPtr<ID3D11BlendState>>();
		state current_state = UNDEFINED;

		ID3D11BlendState* get(state state) {
			auto it = blend_states.find(state);
			if (it != blend_states.end()) { return it->second.Get(); }

			HRESULT hr{ S_OK };
			D3D11_BLEND_DESC blend_desc{};

			blend_desc.AlphaToCoverageEnable = FALSE;
			blend_desc.IndependentBlendEnable = TRUE;
			blend_desc.RenderTarget[0].BlendEnable = TRUE;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			switch (state) {
			case NONE:
			{
				blend_desc.IndependentBlendEnable = FALSE;
				blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
				blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				break;
			}
			case ALPHA:
			{
				blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
				blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				break;
			}
			case ADD:
			{
				blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
				blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				break;
			}
			case SUBTRACT:
			{
				blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
				blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
				blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				break;
			}
			case REPLACE:
			{
				blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
				blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
				blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

				break;
			}
			case MULTIPLY:
			{
				blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
				blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
				blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
				blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
				blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				break;
			}
			case LIGHTEN:
			{
				blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MAX;
				blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
				break;
			}
			case DARKEN:
			{
				blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MIN;
				blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MIN;
				break;
			}
			case SCREEN:
			{
				blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
				blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
				blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				break;
			}
			case MASK: 
			{
				blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
				blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
				blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_COLOR;
				blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				break;
			}
			default:
				_ASSERT_EXPR(false, L"Invalid Blend State");
			}

			blend_states.insert(std::make_pair(state, ComPtr<ID3D11BlendState>()));
			auto jt = blend_states.find(state);
			hr = device::get()->CreateBlendState(&blend_desc, jt->second.GetAddressOf()); VERIFY;

			return jt->second.Get();
		}

		void set(state state) {
			if (state == current_state) return;

			current_state = state;
			device::context()->OMSetBlendState(get(state), nullptr, 0xFFFFFF);
		}
	}
}