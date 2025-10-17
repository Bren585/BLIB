#pragma once
#include "geometric_primitive.h"
#include "object.h"
#include "canvas.h"

namespace BLIB {

	class billboard : public full::object {
	private:
		quad* quad_ptr = nullptr;
		bool dynamic;

	public:
		billboard(							bool dynamic = true) : dynamic	(dynamic) { set_model(create_quad());	}
		billboard(const wchar_t* filename,	bool dynamic = true) : billboard(dynamic) { set_texture(filename);		}
		billboard(const sprite* spr,		bool dynamic = true) : billboard(dynamic) { set_texture(spr);			}
		virtual ~billboard() = default;

		void set_texture(const wchar_t* filename);
		void set_texture(const sprite* spr);
		void set_dynamic(bool on = true);
		void _basic_update(float elapsed_time) override;
		void _render() const override;

	};

}