#include "pch.h"
#include "manager.h"
#include "scene.h"
#include "transition_scene.h"
#include "load_scene.h"

using std::unordered_map;
using std::vector;
using std::unique_ptr;

namespace BLIB {
	using generic::scene;

	namespace manager {

		unordered_map<task_id, unique_ptr<status>> tasks;

		task_id		scene_stack[scene_stack_size] = { 0 };

		task_id		id_counter		= 1;
		bool		stop			= false;

		namespace _private {
			unordered_map<task_id, unique_ptr<status>>& get_tasks() { return tasks; }
		}

		//static scene* get_scene(task_id id) {
		//	auto scene_ptr = tasks.find(id);
		//	if (scene_ptr == tasks.end()) { return nullptr; }
		//	return dynamic_cast<scene*>(tasks[id].get()); 
		//}

		static void end_task(task_id id) {
			tasks[id]->kill();
			tasks.erase(id);
			for (int slot = 0; slot < scene_stack_size; slot++) {
				if (scene_stack[slot] == id) { scene_stack[slot] = 0; }
			}
		}

		bool tick(float elapsed_time) {
			if (stop) return false;

			bool active = false;
			vector<task_id> clean_up;

			for (auto& task : tasks) {
				status::activity report = task.second->report();
				if (report == status::finished	) { clean_up.push_back(task.first); }
				else { active = true;  }
				task.second->tick(elapsed_time);
			}

			for (task_id id : clean_up) {
				if (tasks[id]->is_preserved()) continue;
				end_task(id);
			}

			return active;
		}

		void resize(float2 size) {
			for (int i = 0; i < scene_stack_size; i++) {
				scene* scene_cast = get_scene(scene_stack[i]);
				if (scene_cast) { scene_cast->resize(size); }
			}
		}

		void end() {
			stop = true;
		}

		void kill() {
			bool pending;
			do {
				pending = false;
				for (auto& task : tasks) {
					task.second->try_end_load();
					if (task.second->report() == status::unloaded) { pending = true; }
				}
				if (pending) std::this_thread::sleep_for(std::chrono::milliseconds(1));
			} while (pending);

			for (auto& task : tasks) {
				task.second->unpreserve(task.second.get());
				task.second->kill();
			}
			tasks.clear();
			end();
		}

		task_id add(status* s) {
			if (s->init_id(id_counter)) { id_counter++; }
			tasks.emplace(s->get_id(), unique_ptr<status>(s));
			return s->get_id();
		}

		//task_id add(scene* s) {
		//	task_id new_id = id_counter++;
		//	tasks.insert({ new_id, unique_ptr<status>(s) });
		//	s->init_id(new_id);
		//	return new_id;
		//}

		void stage(task_id id, int slot, transition t, float duration) {
			generic::scene* stage_scene = get_scene(id);
			assert(stage_scene != nullptr);

			if (stage_scene->report() == status::unloaded) {
				stage_scene = get_scene(add(new load_scene(id, slot, t, duration)));
			}

			if (t != transition::none) {
				add_and_stage(new transition_scene(stage_scene, get_scene(scene_stack[slot]), slot, t, duration), slot);
			}
			else {
				if (scene_stack[slot]) unstage(scene_stack[slot]);
				scene_stack[slot] = stage_scene->get_id();
				stage_scene->resize(window::size());
				stage_scene->wake();
				stage_scene->preserve(stage_scene);
			}
		}

		task_id add_and_stage(generic::scene* s, int slot, transition t, float duration) {
			task_id new_id = add(s);
			stage(new_id, slot, t, duration);
			return new_id;
		}

		int unstage(task_id id, transition t, float duration) {
			scene* scene_ptr = get_scene(id);
			assert(scene_ptr);
			for (int slot = 0; slot < scene_stack_size; slot++) {
				if (scene_stack[slot] == id) {
					if (t != transition::none) {
						add_and_stage(new transition_scene(nullptr, scene_ptr, slot, t, duration), slot);
					}
					else {
						scene_stack[slot] = 0;
						scene_ptr->unpreserve(scene_ptr);
						scene_ptr->sleep();
					}
					return slot;
				}
			}
			return -1;
		}

		void display() {
			for (int i = scene_stack_size - 1; i >= 0; i--) {
				scene* scene_ptr = get_scene(scene_stack[i]);
				if (scene_ptr && scene_ptr->report() != status::unloaded) scene_ptr->render(); 
			}
		}

		status* get_task(task_id id) {
			auto task = tasks.find(id);
			return (task != tasks.end()) ? task->second.get() : nullptr;
		}

		generic::scene* get_scene(task_id id) {
			return dynamic_cast<generic::scene*>(get_task(id)); 
		}

		generic::scene* get_slot(int slot) {
			assert(slot >= 0 && slot < scene_stack_size);
			return get_scene(scene_stack[slot]);
		}

	}

}