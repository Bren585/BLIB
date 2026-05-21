#pragma once

namespace BLIB {
	namespace sampler {
		enum state {
			POINT,
			CLAMP_POINT,
			LINEAR,
			ANISOTROPIC,
			COMPARE,

			SAMPLER_COUNT,
			UNDEFINED,
			DEFAULT = ANISOTROPIC
		};

		void set(state s = DEFAULT, int slot = 0);

		void release_all();
	}
}