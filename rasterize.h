#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <map>

namespace BLIB {

	namespace rasterize {

		enum state {
			FILL = 0,
			FILL_FLIP,
			WIRE,
			WIRE_FLIP,
			WIRE_FRONT,
			WIRE_FRONT_FLIP,

			COUNT,
			DEFAULT = FILL,
			UNDEFINED = -1
		};

		//ID3D11RasterizerState* get(state state);

		void set(state state = DEFAULT);

		void release_all();
	}
}