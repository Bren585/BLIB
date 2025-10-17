#pragma once
#include "string.h"
#define AUDIO_EXT L".wav"

namespace BLIB {

	enum audio_control {
		add,
		set,
		mlt,
	};

	enum audio_setting {
		volume,
		pitch,
		pan
	};

	namespace audio {

		enum {
			unset = -2,
			all_tracks = -1
		};

		void init();
		void update();
		void uninit();

		void set_filepath(string path);

		void load		(string filename);
		void unload		(string filename = L"");

		int  play		(string filename, bool loop = false);
		void pause		(int instance = all_tracks);
		void resume		(int instance = all_tracks);
		void stop		(int instance = all_tracks);

		bool is_looped	(int instance = all_tracks);
		bool is_playing	(int instance = all_tracks);
		bool is_paused	(int instance = all_tracks);
		bool is_stopped	(int instance = all_tracks);

		void config(string filename, audio_control op, audio_setting target, float val);

	}

}