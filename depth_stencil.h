#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <map>

namespace BLIB {

	namespace stencil {

		enum state {
			DEPTH_MASK = 0,
			DEPTH_NONE,
			SURFACE_MASK,
			SURFACE_NONE,

			COUNT,
			DEFAULT = DEPTH_MASK,
			UNDEFINED = -1
		};

		ID3D11DepthStencilState* get(state state);

		void set(state state = DEFAULT);

		void release_all();
	}
}