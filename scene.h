#pragma once

#include "status.h"
#include "canvas.h"
#include "window.h"
#include "manager.h"

typedef unsigned int scene_id;

namespace BLIB {

	class scene : public status, public canvas {
		friend scene_id manager::add(scene*);
	private:
		scene_id id = 0;

	protected:
		virtual void draw() {}

	public:
		scene() : canvas(window::size()) { pivot = C_BL; }
		scene_id get_id() const { return id; }

		virtual ~scene() = default;
		virtual void init() override = 0;
		virtual void update(float elapsed_time) override = 0;
		virtual void idle(float elapsed_time) override = 0;
		//virtual void kill() {}
		//virtual void wake() {}

		void resize(float2 size) { canvas::resize(size); on_resize(); }
		virtual void on_resize() {}

		inline void render() {
			RENDER_LOCK;
			canvas::focus();
			canvas::clear();
			draw();
			canvas::render_to_main();
		}
	};

}