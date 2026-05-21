#pragma once
#include "BLIB.h"
#include "math.h"

namespace BLIB {

	class status;
	namespace generic { class scene; }

	typedef unsigned int task_id;

	enum class transition {
		none,
		fade
	};

	namespace manager {

		namespace _private {
			std::unordered_map<task_id, std::unique_ptr<status>>& get_tasks();
		}

		constexpr int scene_stack_size = 4;

		bool tick	(float elapsed_time);
		void resize	(float2 size);
		void kill	(); // End immedietly, cleanup
		//void end	(); // Exit process

		//task_id	add(generic::scene* s);

		task_id		add				(status* s);
		void		stage			(task_id id, int slot, transition t = transition::none, float duration = 0.0f);
		task_id		add_and_stage	(generic::scene* s, int slot, transition t = transition::none, float duration = 0.0f);
		int			unstage			(task_id id, transition t = transition::none, float duration = 0.0f);
		// swap		(int to_layer, int from_layer)
		
		template <class S>
		task_id find_first_of_type() {
			for (auto& pair : _private::get_tasks()) {
				if (dynamic_cast<S*>(pair.second.get())) {
					return pair.first;
				}
			}
			return 0;
		}

		void display();

		status*							get_task	(task_id id);
		generic::scene*					get_scene	(task_id id);
		generic::scene*					get_slot	(int slot);

		inline const status*			peek_task	(task_id id)	{ return get_task(id); }
		inline const generic::scene*	peek_scene	(task_id id)	{ return get_scene(id); }
		inline const generic::scene*	peek_slot	(int slot)		{ return get_slot(slot); }

	}

}