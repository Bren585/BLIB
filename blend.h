#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <map>

namespace BLIB {

	namespace blend {

		enum state {
			NONE = 0,
			ALPHA,
			ADD,
			SUBTRACT,
			REPLACE,
			MULTIPLY,
			LIGHTEN,
			DARKEN,
			SCREEN,
			MASK,

			COUNT,
			DEFAULT = ALPHA,
			UNDEFINED = -1
		};

		ID3D11BlendState* get(state state);

		void set(state state = DEFAULT);

		void release_all();
	}
}