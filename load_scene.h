#pragma once
#include "manager.h"
#include "entity.h"
#include "scene.h"

namespace BLIB {

	class load_scene_any {
	protected:
		inline static color  background_color = BLACK;
		inline static string background_filename = L"-1";
		inline static string load_icon_filename = L"-1";
		inline static string load_text = "Loading...";

	public:
		static void set_background	(color c)		{ background_color = c; }
		static void set_background	(string file)	{ background_filename = file; }
		static void set_load_icon	(string file)	{ load_icon_filename = file; }
		static void set_text		(string text)	{ load_text = text; }

	};

	template <class S>
	class load_scene : public scene, load_scene_any {
	private:
		scene_id scene_id;
		int slot;
		flat::object background;
		flat::object load_icon;

		transition	exit_transition;
		float		exit_duration;

	public:
		template<typename... Args>
		load_scene(int slot, transition t = transition::none, float duration = 0, Args&&... args) : slot(slot), scene_id(manager::add(new S(std::forward<Args>(args)...))), exit_transition(t), exit_duration(duration) { canvas::set_background(background_color); }

		inline void init() override {

			if (background_filename != L"-1") {
				background.load_sprite(background_filename);
				background.pivot = C_BL;
				background.size = window::size().x;
			}

			if (load_icon_filename != L"-1") {
				load_icon.load_sprite(load_icon_filename);
				load_icon.pivot = C_CC;
				load_icon.size = 100;
				load_icon.pos = { window::size().x - 100.0f, 100.0f };
			}
		}

		void update(float elapsed_time)	override { if (manager::peek(scene_id)->report() == status::active) { manager::stage(scene_id, slot, exit_transition, exit_duration); state = complete; } idle(elapsed_time); }
		void idle(float elapsed_time)	override { load_icon.angle += 60 * elapsed_time; }
		void draw()						override { background.render(); text::out(load_text, 0, 100, FONT_3); load_icon.render(); }
		void kill()						override { /*if (manager::peek(scene_id)->report() != status::active) manager::kill(scene_id);*/ }

	};

}