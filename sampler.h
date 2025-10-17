#pragma once

namespace BLIB {
	namespace sampler {
		enum state {
			POINT,
			LINEAR,
			ANISOTROPIC,

			SAMPLER_COUNT,
			UNDEFINED,
			DEFAULT = ANISOTROPIC
		};

		void set(state s = DEFAULT, int slot = 0);
	}
}