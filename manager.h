#pragma once
#include "BLIB.h"
#include "math.h"

// Create a stack of scenes
// Create a "transition scene" that takes two scenes and does the grapical transition before degenerating into the destination scene
// -- This can only be done by telling the manager "hey, replace with with this scene"

namespace BLIB {

	class status;
	class scene;

	typedef unsigned int status_id;
	typedef unsigned int scene_id;

	enum class transition {
		none,
		fade
	};

	namespace manager {

		namespace _private {
			auto& get_tasks();
		}

		constexpr int scene_stack_size = 4;

		bool tick(float elapsed_time);
		void resize(float2 size);
		void kill(); // End immedietly, cleanup
		void end(); // Exit process

		status_id	add(status* s);
		scene_id	add(scene* s);

		void		stage(scene_id id, int slot, transition t = transition::none, float duration = 0.0f);
		scene_id	add_and_stage(scene* s, int slot, transition t = transition::none, float duration = 0.0f);
		int			unstage(scene_id id, transition t = transition::none, float duration = 0.0f);
		// swap		(int to_layer, int from_layer)
		
		template <class S>
		status_id find_first_of_type() {
			for (auto& pair : _private::get_tasks()) {
				if (dynamic_cast<S*>(pair.second.get())) {
					return pair.first;
				}
			}
			return 0;
		}

		void display();

		const status* peek(status_id id);

	}

}