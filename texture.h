#pragma once
#include <d3d11.h>
#include "string.h"
#include "math.h"

namespace BLIB {
	namespace texture {
		HRESULT load_file(const wchar_t* filename, ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc);
		inline HRESULT load_file(const string& filename, ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc) { return load_file((filename).wide(), shader_resource_view, texture2d_desc); }

		HRESULT make_dummy(ID3D11ShaderResourceView** shader_resource_view, color value, float2 dimension);

		HRESULT save_to_file(ID3D11Resource* resource, string filename);

		HRESULT save_to_file(ID3D11ShaderResourceView* shader_resource_view, string filename);

		void release_all();
	}
}

