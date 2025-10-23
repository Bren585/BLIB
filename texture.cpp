#include "pch.h"
#include "texture.h"

#include <WICTextureLoader.h>

namespace BLIB {
	namespace texture {
		using Microsoft::WRL::ComPtr;
		using std::wstring;
		using std::map;
		using std::make_pair;

		static map<wstring, ComPtr<ID3D11ShaderResourceView>> resources;

		HRESULT load_file(const wchar_t* filename, ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc) {
			HRESULT hr{ S_OK };
			ComPtr<ID3D11Resource> resource;

			auto it = resources.find(filename);
			if (it != resources.end()) {
				it->second.CopyTo(shader_resource_view);
				(*shader_resource_view)->GetResource(resource.GetAddressOf());
			}
			else {
				ComPtr<ID3D11ShaderResourceView> view;
				hr = DirectX::CreateWICTextureFromFile(device::get(), filename, resource.GetAddressOf(), view.GetAddressOf()); VERIFY;
				resources.insert(make_pair(filename, view));
				view.CopyTo(shader_resource_view);
			}

			ComPtr<ID3D11Texture2D> texture2d;
			hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf()); VERIFY;
			texture2d->GetDesc(texture2d_desc);

			return hr;
		}

		HRESULT make_dummy(ID3D11ShaderResourceView** shader_resource_view, color value, float2 dimension) {
			HRESULT hr{ S_OK };

			D3D11_TEXTURE2D_DESC texture2d_desc{};
			texture2d_desc.Width = static_cast<UINT>(dimension.x);
			texture2d_desc.Height = static_cast<UINT>(dimension.y);
			texture2d_desc.MipLevels = 1;
			texture2d_desc.ArraySize = 1;
			texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			texture2d_desc.SampleDesc.Count = 1;
			texture2d_desc.SampleDesc.Quality = 0;
			texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
			texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			size_t texels = texture2d_desc.Width * texture2d_desc.Height;
			std::unique_ptr<hex_rgba[]> sysmem{ std::make_unique<hex_rgba[]>(texels) };
			for (size_t i = 0; i < texels; i++) { sysmem[i] = value.abgr(); }

			D3D11_SUBRESOURCE_DATA subresource_data{};
			subresource_data.pSysMem = sysmem.get();
			subresource_data.SysMemPitch = sizeof(hex_rgba) * static_cast<UINT>(dimension.x);

			ComPtr<ID3D11Texture2D> texture2d;
			hr = device::get()->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d); VERIFY;

			D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
			shader_resource_view_desc.Format = texture2d_desc.Format;
			shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shader_resource_view_desc.Texture2D.MipLevels = 1;
			hr = device::get()->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc, shader_resource_view); VERIFY;

			return hr;
		}

		void release_all() { resources.clear(); }
	}
}