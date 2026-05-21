#pragma once
#include <vector>
#include "math.h"
#include "string.h"

namespace BLIB {

	struct animation {

		struct keyframe {
			struct node {
				float4x4 global_transform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
				transform local_transform;

				SERIALIZE(global_transform, local_transform)
			};
			std::vector<node> nodes;
		
			node&		at(int i)		{ return nodes[i]; }
			const node& at(int i) const { return nodes[i]; }

			SERIALIZE(nodes)

		};
		std::vector<keyframe> sequence;
		string name;
		float sampling_rate{ 0 }; // A value of 0 will be set to the default sampling rate

		SERIALIZE(name, sampling_rate, sequence)

	private:
		float	timer			{ 0.0f	};
		bool	loop			{ false };
		float	playback_speed	{ 1.0f	};

	public:
		inline bool update(float elapsed_time) {
			timer += elapsed_time * playback_speed;
			float duration = get_duration();
			if (timer > duration) {
				if (loop) timer -= duration;
				return loop;
			}
			return true;
		}
		inline void reset		(				) { timer = 0; }
		inline void set_loop	(bool l = true	) { loop = l; }
		inline void set_playback(float speed = 1) { playback_speed = (speed > 0 ? speed : 1); }

		inline float			get_duration		(				) const { return static_cast<float>(sequence.size()) / sampling_rate;					}
		inline float			get_play_duration	(				) const { return static_cast<float>(sequence.size()) / sampling_rate / playback_speed;	}
		inline float			get_timer			(				) const { return timer;																	}
		inline float			get_remaining_time	(				) const { return get_duration() - get_timer();											}
		inline int				get_frame			(float time		) const { return static_cast<int>(time * sampling_rate);								}
		inline int				get_valid_frame		(float time		) const { return clamp(0, get_frame(time), static_cast<int>(sequence.size()) - 1);		}
		inline const keyframe&	get_keyframe		(float time = -1) const { return sequence[get_valid_frame((time == -1) ? timer : time)];				}
		inline bool				is_loop				(				) const { return loop;																	}
		inline bool				is_active			(				) const { return loop || timer < get_duration();										}

	};

	inline void blend_keyframes(const animation::keyframe& in_key_1, const animation::keyframe& in_key_2, float factor, animation::keyframe& out_key) {
		const size_t node_count{ in_key_1.nodes.size() };
		out_key.nodes.clear();
		out_key.nodes.resize(node_count);
		for (size_t node_index = 0; node_index < node_count; node_index++) {
			out_key.nodes[node_index].global_transform = lerp(in_key_1.nodes[node_index].global_transform, in_key_2.nodes[node_index].global_transform, factor);
		}
	}
}