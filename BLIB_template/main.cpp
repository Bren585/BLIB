#include <time.h>

#include "BLIB/BLIB.h"
using namespace BLIB;

#ifdef _DEBUG
#include "BLIB/device.h"
#endif

#include "all_scenes.h"

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_  HINSTANCE prev_instance, _In_ LPSTR cmd_line, _In_ int cmd_show)
{
	srand(static_cast<unsigned int>(time(nullptr)));

	if (!init(instance, prev_instance, cmd_line, cmd_show)) { return 0; }

#ifdef _DEBUG
	device::debug(); // Enable Debug Layer
#endif

	text::	set_filepath(L"data/fonts/");
	audio::	set_filepath(L"data/audio/");
	sprite::set_filepath(L"data/images/");
	model::	set_filepath(L"data/models/");
	shader::set_filepath(L"data/shaders/");

	//load_scene_any::set_background(L"background.png");
	//load_scene_any::set_load_icon(L"white.png"); 

	manager::add_and_stage(/*new my_scene()*/ nullptr, 0);

	while (loop());

	uninit();

#ifdef _DEBUG
	//device::report_live();
#endif

	return 0;
}