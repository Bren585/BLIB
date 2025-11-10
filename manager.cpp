#include "pch.h"
#include "manager.h"
#include "scene.h"
#include "transition_scene.h"

using std::unordered_map;
using std::vector;
using std::unique_ptr;

namespace BLIB {
	using generic::scene;

	namespace manager {

		unordered_map<status_id, unique_ptr<status>> tasks;

		scene_id scene_stack[scene_stack_size] = { 0 };

		status_id	id_counter		= 1;
		bool		stop			= false;

		namespace _private {
			auto& get_tasks() { return tasks; }
		}

		static scene* get_scene(scene_id id) { 
			auto scene_ptr = tasks.find(id);
			if (scene_ptr == tasks.end()) { return nullptr; }
			return dynamic_cast<scene*>(tasks[id].get()); 
		}

		static void end_task(status_id id) {
			tasks[id]->kill();
			tasks.erase(id);
		}

		bool tick(float elapsed_time) {
			if (stop) return false;

			bool active = false;
			vector<status_id> clean_up;

			for (auto& task : tasks) {
				task.second->tick(elapsed_time);
				if (task.second->report() != status::complete) { active = true; }
				else { clean_up.push_back(task.first); }
			}

			for (status_id id : clean_up) {
				if (tasks[id]->is_preserved()) continue;
				end_task(id);
			}

			return active;
		}

		void resize(float2 size) {
			for (auto& task : tasks) {
				scene* scene_cast = dynamic_cast<BLIB::scene*>(task.second.get());
				if (scene_cast) { scene_cast->resize(size); }
			}
		}

		void kill() {
			for (auto& task : tasks) {
				task.second->kill();
			}
			tasks.clear();
			end();
		}

		void end() {
			stop = true;
		}

		status_id add(status* s) {
			int new_id = id_counter++;
			tasks.insert({ new_id, unique_ptr<status>(s) });
			return new_id;
		}

		scene_id add(scene* s) {
			scene_id new_id = id_counter++;
			tasks.insert({ new_id, unique_ptr<status>(s) });
			s->id = new_id;
			return new_id;
		}

		void stage(scene_id id, int slot, transition t, float duration) {
			scene* stage_scene = get_scene(id);
			assert(stage_scene != nullptr);

			if (t != transition::none) {
				add_and_stage(new transition_scene(stage_scene, get_scene(scene_stack[slot]), slot, t, duration), slot);
			}
			else {
				scene_stack[slot] = id;
				stage_scene->wake();
				stage_scene->preserve();
			}
		}

		scene_id add_and_stage(scene* s, int slot, transition t, float duration) {
			scene_id new_id = add(s);
			stage(new_id, slot, t, duration);
			return new_id;
		}

		int unstage(scene_id id, transition t, float duration) {
			scene* scene_ptr = get_scene(id);
			assert(scene_ptr);
			for (int i = 0; i < scene_stack_size; i++) {
				if (scene_stack[i] == id) {
					scene_stack[i] = 0;
					scene_ptr->unpreserve();
					return i;
				}
			}
			return -1;
		}

		void display() {
			for (int i = scene_stack_size - 1; i >= 0; i--) {
				scene* scene_ptr = get_scene(scene_stack[i]);
				if (scene_ptr) scene_ptr->render(); 
			}
		}

		const status* peek(status_id id) {
			auto task = tasks.find(id);
			return (task != tasks.end()) ? task->second.get() : nullptr;
		}

	}

}