#pragma once
#include "scene.h"
#include "manager.h"

namespace BLIB {

	class transition_scene : public flat::scene {
	private:
		generic::scene* to_scene;
		generic::scene* from_scene;

		task_id to_scene_id = 0;
		task_id from_scene_id = 0;

		const int slot;

		const transition t;

		const float duration;

	public:
		transition_scene(generic::scene* to, generic::scene* from, int slot, transition t, float duration) : to_scene(to), from_scene(from), slot(slot), t(t), duration(duration) { assert(t != transition::none && (to || from)); to_scene->preserve(); from_scene->preserve(); }

		void init() override {
			if (to_scene)	{ to_scene_id	= to_scene->get_id();	}
			if (from_scene) { from_scene_id = from_scene->get_id(); }

			manager::unstage(to_scene_id);
			manager::unstage(from_scene_id);
		}

		void update	(float elapsed_time)	override { if (timer > duration) { finish(); } }
		void idle	(float elapsed_time)	override {}

		void draw(render_settings) const override {
			switch (t) {
			case transition::none:
				if (to_scene) { to_scene->render(); }
				break;
			case transition::fade:
				generic::scene* fade_scene = to_scene ? to_scene : from_scene;
				fade_scene->tint.a = clamp(0.0f, timer / duration, 1.0f);

				if (from_scene) { from_scene->render(); }
				if (to_scene)	{ to_scene->render(); }
				break;
			}
		} 

		void kill()	override {
			switch (t) {
			case transition::none:
				break;
			case transition::fade:
				if (from_scene) { from_scene->tint.a = 1;   }
				if (to_scene)	{ to_scene->tint.a = 1; }
				break;
			}

			if (from_scene) { from_scene->unpreserve();				}
			if (to_scene)	{ manager::stage(to_scene_id, slot);	}
		}

	};

}