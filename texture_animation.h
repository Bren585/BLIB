#pragma once
#include "math.h"
#include "cereal.h"
#include <vector>
#include <map>
#include <queue>

namespace BLIB {

	struct texture_frame {
		float2	uv_index;
		float	duration;

		SERIALIZE(uv_index, duration)
	};

	struct texture_animation {
	private:
		bool	loop		= false;
		float	timer		= 0;
		int		frame_index = -1;
		string	name;
		std::vector<texture_frame> frames;

	public:
		texture_animation() : name("empty animation") {}
		texture_animation(string name, std::vector<texture_frame> frame_set) : name(name), frames(frame_set) {}

		void start			() { frame_index = 0; }
		void stop			() { frame_index = -1; }
		bool is_animating	() { return frame_index >= 0; }

		void set_loop(bool l) { loop = l; }

		bool update(float elapsed_time) {
			if (frame_index < 0) return false;
			timer += elapsed_time;
			while (timer > frames[frame_index].duration) { 
				timer -= frames[frame_index++].duration; 
				if (frame_index >= frames.size()) {
					if (loop) start();
					else { stop(); break; }
				}
			}
			return is_animating();
		}

		string get_name	() const { return name; }
		float2 get_frame() const { return frames[frame_index].uv_index; }

#ifdef _DEBUG
		auto& imgui_get_frames() { return frames; }
		void imgui_set_name(string s) { name = s; }
#endif

		SERIALIZE(frames)
	};

	class texture_animator {
	private:
		struct queued_animation { string name; bool loop = false; };

		std::unordered_map<string, texture_animation> animations;
		std::queue<queued_animation> animation_queue;

		texture_animation&			current()		{ return animations[animation_queue.front().name]; }
		const texture_animation&	current() const { return animations.at(animation_queue.front().name); }

		void update_loop() { current().set_loop(animation_queue.front().loop); }

	public:
		texture_animator() = default;

		void add_animation(texture_animation animation) { animations.try_emplace(animation.get_name(), animation); }

		bool is_animating() const { return animation_queue.size(); }

		bool next() { animation_queue.pop(); return is_animating(); }

		void stop_all() { if (is_animating()) while (next()); }

		void set_loop(bool l) { if (is_animating()) { current().set_loop(l); } }

		void animate(string animation_name, bool loop = false, bool overwrite = true) {
			bool was_animating = is_animating();
			if (overwrite) { stop_all(); }
			animation_queue.push({ animation_name, loop });
			if (!was_animating || overwrite) { 
				update_loop(); 
				current().start();
			}
		}

		void update(float elapsed_time) {
			if (!is_animating()) return;
			if (!current().update(elapsed_time)) {
				if (next()) {
					update_loop();
					current().start();
				}
			}
		}

		float2 get_frame() const { if (is_animating()) return current().get_frame(); else return float2(0); }

#ifdef _DEBUG
		auto& imgui_get_animations() { return animations; }
#endif

		SERIALIZE(animations)
	};

}