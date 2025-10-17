#pragma once
#include "BLIB/scene.h"

class template_scene : public BLIB::scene {
private:
	void init() override {};
	void update(float elapsed_time) override {};
	void idle(float elapsed_time) override {};
	void draw() override {};

public:
	template_scene() {}
	~template_scene() {}
};