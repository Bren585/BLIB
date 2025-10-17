#pragma once
#include "render_settings.h"

namespace BLIB {
	class renderable {
	public:
		virtual void render(render_settings) const = 0;
		virtual ~renderable() = default;
	};

	class updateable {
	public:
		virtual void update(float) = 0;
		virtual ~updateable() = default;
	};
}