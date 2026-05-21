#pragma once
#include "BLIB/scene.h"

/******************************************************************
	BLIB::status

		A status is a process that will be called every frame it is
	awake. It is awake by default.

	 -- User Defined Virtual Methods -- 
	init()		: Called while being constructed.
	update()	: Called once a frame while awake.
	idle()		: Called once a frame while not awake.
	kill()		: Called just before forced cleanup.
	on_wake()	: Called once when awoken.
	on_sleep()  : Called once when put to sleep.

	 -- Important Library Methods --
	finish()	: Deactivate and mark self for cleanup. 
	wake()		: Wake.
	sleep()		: Sleep.
	get_id()	: Return own ID.

	 -- Important Members -- 
	float timer : Auto increments with elapsed time for your 
					convenience. It is protected, so may be 
					changed or reset at your lesiure. 

******************************************************************/

class template_status : public BLIB::status {
private:

	// Strictly Required for function
	void init() override {}
	void update(float elapsed_time) override {}

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void kill() override {}
	void on_wake() override {}
	void on_sleep() override {}

public:
	template_status() {}
	~template_status() {}
};

/******************************************************************
	BLIB::flat::scene

		Scene inherits from status, but is also called for 
	rendering when staged by the manager. Scenes are asleep by 
	default, but wake when staged.

		Scene also inherits from flat::object, so it can be moved, 
	scaled, rotated, etc. like any other object.

	 -- User Defined Virtual Methods --
	+ status methods (see above)
	draw() const		: Draw to the screen. This may be called 
							multiple times a frame, and so no data 
							changes should be made.
	on_resize()			: Called when the scene or window size changes.

	 -- Important Library Methods --
	+ status methods (see above)
	get_camera()		: Gets the active camera.
	set_camera()		: Sets the active camera.
	get_scene_lights()	: Gets the envirnmental lights.
	set_scene_lights()	: Sets the envirnmental lights.
	get_lights()		: Gets the spotlights.
	set_lights()		: Sets the spotlights.

******************************************************************/

class template_flat_scene : public BLIB::flat::scene {
private:
	// Strictly Required for function
	void init() override {}
	void update(float elapsed_time) override {}
	void draw(BLIB::render_settings) const override {}

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}

	// Optional
	void kill() override {}
	void on_wake() override {}
	void on_sleep() override {}

public:
	template_flat_scene() {}
	~template_flat_scene() {}
};

/******************************************************************
	BLIB::full::scene

		A full scene has one important distinction from flat scenes,
	they used deferred rendering. This means the rendering pipeline 
	is slightly different, and two separate draw functions are
	available.

	 -- User Defined Virtual Methods --
	+ scene methods (see above)
	draw() const		: Draw Opaque geometry that casts shadows.
	draw_transparent()	: Draw Transparent geometry, or otherwise
							geometry that doesn't cast shadows. (UI)

******************************************************************/

class template_full_scene : public BLIB::full::scene {
private:
	// Strictly Required for function
	void init() override {}
	void update(float elapsed_time) override {}
	void draw(BLIB::render_settings) const override {}

	// Required, but {} is acceptable
	void idle(float elapsed_time) override {}
	void draw_transparent() const override {}

	// Optional
	void kill() override {}
	void on_wake() override {}
	void on_sleep() override {}

public:
	template_full_scene() {}
	~template_full_scene() {}
};