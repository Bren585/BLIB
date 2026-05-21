#include "pch.h"
#include "audio.h"
#include "DirectXTK-main/Inc/Audio.h"

using std::map;

namespace BLIB {
	namespace audio {

		std::unique_ptr<DirectX::AudioEngine> AE = nullptr;

		struct audio_source {
			std::unique_ptr<DirectX::SoundEffect> SE;
			float volume	= 1.0f;
			float pitch		= 0.0f;
			float pan		= 0.0f;
		};

		struct audio_instance {
		public:
			audio_source* parent;
			std::unique_ptr<DirectX::SoundEffectInstance> SEI;
			float fade_duration = 0;
			float fade_timer = 0;
			float base_volume = 1;

			enum fade_type {
				no_fade = 0,
				fade_in,
				fade_out
			} fade = fade_in;

			enum on_fade_out_type {
				fade_pause,
				fade_stop
			} on_fade_out = fade_stop;

			void update(float elapsed_time) {
				if (fade) {
					fade_timer += elapsed_time;
					if (fade_timer > fade_duration) { 
						if (fade_in)	{ SEI->SetVolume(base_volume); }
						else			{ if (on_fade_out == fade_stop) SEI->Stop(); else SEI->Pause(); }
						fade = no_fade;
					}
					else {
						float t = fade_timer / fade_duration;
						if (fade_out) { t = 1 - t; }
						SEI->SetVolume(base_volume * t);
					}
				}
			}

		};
		
		string filepath = L"";
		void set_filepath(string path) { filepath = path; }

		map<string, audio_source> sources = map<string, audio_source>();

		int tracks_cursor = 0;
		constexpr int max_tracks = 99;
		map<int, audio_instance> tracks = map<int, audio_instance>();

		void init() {
			AE = std::make_unique<DirectX::AudioEngine>();
		}

		void update(float elapsed_time) {
			AE->Update();
			for (auto it = tracks.begin(); it != tracks.end(); ) {
				it->second.update(elapsed_time);
				if (it->second.SEI->GetState() == DirectX::STOPPED) { it = tracks.erase(it); }
				else { ++it; }
			}
		}

		void uninit() {
			stop();
			tracks.clear();
			sources.clear();
		}

		void load(string filename) {
			_ASSERT_EXPR_A(filepath != L"",  L"Must Set Filepath");

			if (sources.find(filename) == sources.end()) {
				sources[filename].SE = std::make_unique<DirectX::SoundEffect>(AE.get(), filepath + filename + AUDIO_EXT);
			}
		}

		void unload(string filename) {
			if (filename == L"") {
				sources.clear();
				return;
			}

			if (sources.find(filename) != sources.end()) {
				sources.erase(filename);
			}
		}

		int play(string filename, float fade_time, bool loop) {
			load(filename);

			auto source = sources.find(filename);

			int start = tracks_cursor - 1;
			if (start == -1) start = max_tracks;
			while (tracks.find(tracks_cursor) != tracks.end() && tracks_cursor != start) { ++tracks_cursor; if (tracks_cursor > max_tracks) tracks_cursor = 0; }
			if (tracks_cursor == start && tracks.find(tracks_cursor) != tracks.end()) return unset;

			int id = tracks_cursor++;
			if (tracks_cursor > max_tracks) tracks_cursor = 0;

			tracks[id].parent = &source->second;
			tracks[id].SEI = source->second.SE->CreateInstance();

			tracks[id].base_volume = source->second.volume;
			tracks[id].SEI->SetPitch(source->second.pitch);
			tracks[id].SEI->SetPan(source->second.pan);

			tracks[id].fade = audio_instance::fade_in;
			tracks[id].fade_duration = fade_time;
			tracks[id].fade_timer = 0;

			tracks[id].SEI->Play(loop);

			return id;
		}

		void pause(int instance, float fade_time) {
			if (instance == -1) {
				for (auto& track : tracks) {
					track.second.fade = audio_instance::fade_out;
					track.second.on_fade_out = audio_instance::fade_pause;
					track.second.fade_duration = fade_time;
					track.second.fade_timer = 0;
				}
			}
			else {
				auto track = tracks.find(instance);
				if (track != tracks.end()) {
					track->second.fade = audio_instance::fade_out;
					track->second.on_fade_out = audio_instance::fade_pause;
					track->second.fade_duration = fade_time;
					track->second.fade_timer = 0;
				}
			}
		}

		void resume(int instance, float fade_time) {
			if (instance == -1) {
				for (auto& track : tracks) {
					track.second.SEI->Resume();
					track.second.fade = audio_instance::fade_in;
					track.second.fade_duration = fade_time;
					track.second.fade_timer = 0;
				}
			}
			else {
				auto track = tracks.find(instance);
				if (track != tracks.end()) {
					track->second.SEI->Resume();
					track->second.fade = audio_instance::fade_in;
					track->second.fade_duration = fade_time;
					track->second.fade_timer = 0;
				}
			}
		}

		void stop(int instance, float fade_time) {
			if (instance == -1) {
				for (auto& track : tracks) {
					track.second.fade = audio_instance::fade_out;
					track.second.on_fade_out = audio_instance::fade_stop;
					track.second.fade_duration = fade_time;
					track.second.fade_timer = 0;
				}
			}
			else {
				auto track = tracks.find(instance);
				if (track != tracks.end()) {
					track->second.SEI->Stop();
					track->second.fade = audio_instance::fade_out;
					track->second.on_fade_out = audio_instance::fade_stop;
					track->second.fade_duration = fade_time;
					track->second.fade_timer = 0;
				}
			}
		}

		bool is_looped(int instance) {
			if (instance == -1) {
				for (auto& track : tracks) {
					if (track.second.SEI->IsLooped()) return true;
				}
				return false;
			}
			else {
				auto track = tracks.find(instance);
				if (track != tracks.end()) {
					return track->second.SEI->IsLooped();
				}
				return false;
			}
		}

		bool is_playing(int instance) {
			if (instance == -1) {
				for (auto& track : tracks) {
					if (track.second.SEI->GetState() == DirectX::PLAYING) return true;
				}
				return false;
			}
			else {
				auto track = tracks.find(instance);
				if (track != tracks.end()) {
					return track->second.SEI->GetState() == DirectX::PLAYING;
				}
				return false;
			}
		}

		bool is_paused(int instance) {
			if (instance == -1) {
				for (auto& track : tracks) {
					if (track.second.SEI->GetState() == DirectX::PAUSED) return true;
				}
				return false;
			}
			else {
				auto track = tracks.find(instance);
				if (track != tracks.end()) {
					return track->second.SEI->GetState() == DirectX::PAUSED;
				}
				return false;
			}
		}

		bool is_stopped(int instance) {
			if (instance == -1) {
				for (auto& track : tracks) {
					if (track.second.SEI->GetState() != DirectX::STOPPED) return false;
				}
				return true;
			}
			else {
				auto track = tracks.find(instance);
				if (track != tracks.end()) {
					return track->second.SEI->GetState() == DirectX::STOPPED;
				}
				return true;
			}
		}

		void config(string filename, audio_control op, audio_setting target, float val) {
			load(filename);

			float* victim;

			switch (target) {
			case volume:	victim = &sources[filename].volume; break;
			case pitch:		victim = &sources[filename].pitch;	break;
			case pan:		victim = &sources[filename].pan;	break;
			default: return;
			}

			switch (op) {
			case add: *victim += val; break;
			case set: *victim  = val; break;
			case mlt: *victim *= val; break;
			default: return;
			}
		}
	}
}
