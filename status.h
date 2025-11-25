#pragma once
#include "verify.h"
#include <thread>
#include <atomic>
#include <windows.h>
#include "manager.h"

namespace BLIB {

	class status {
	public:
		enum activity {
			unloaded,
			inactive,
			active,
			finished
		};

	private:
		std::thread*		loader = nullptr;
		std::atomic<bool>	loaded { false };

		bool	_preserve	= false;
		task_id id			= 0;

	protected:
		activity	state	= unloaded;
		float		timer	= 0;

		virtual void	init		() {}
		void			coinit		() { srand(static_cast<unsigned int>(time(nullptr))); HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED); VERIFY; init(); CoUninitialize(); loaded.store(true, std::memory_order_release); }
		void			finish		() { state = finished; }

		virtual void	on_load		() { wake(); }
		virtual void	on_wake		() {}
		virtual void	on_sleep	() {}

	public:
		virtual ~status() { if (loader) { while(!loader->joinable()); loader->join(); delete loader; } }//{ _ASSERT_EXPR(loader == nullptr, L"Status deleted improperly"); }

		inline virtual void tick(float elapsed_time) { 
			timer += elapsed_time; 
			switch (state) { 
			case unloaded:	if (loader) {
								if (loader->joinable() && loaded.load(std::memory_order_acquire)) { 
									loader->join(); on_load(); delete loader; loader = nullptr;
								} 
							}
							else if (!loader) {loader = new std::thread(&status::coinit, this); }		break;
			case active:	update(elapsed_time);														break;
			default:		idle(elapsed_time);															break;
			}
		};

		void			wake	()	{ if (state == finished) { return; } state = active;	on_wake();	}
		void			sleep	()	{ if (state == finished) { return; } state = inactive;	on_sleep(); }
		virtual void	kill	()	{} // Ubruptly end the task because I'm about to delete you

		virtual void update	(float elapsed_time) { finish(); }
		virtual void idle	(float elapsed_time) {}

		activity report() const { return state; }

		void preserve		()			{ _preserve = true; }
		void unpreserve		()			{ _preserve = false; }
		bool is_preserved	() const	{ return _preserve; }

		task_id get_id	()				{ return id; }
		bool	init_id	(task_id tid)	{ if (!id) { id = tid; return true; } else { return false; } }
	};

}