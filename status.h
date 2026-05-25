#pragma once
#include "verify.h"
#include <thread>
#include <atomic>
#include <windows.h>
#include <set>
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
		std::set<void*>		preservatives{};

		task_id		id			= 0;
		activity	state		= unloaded;

	protected:
		float		timer		= 0;

		virtual void	init		() {}
		void			coinit		() { srand(static_cast<unsigned int>(time(nullptr))); HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED); VERIFY; init(); CoUninitialize(); loaded.store(true, std::memory_order_release); }
		void			finish		() { state = finished; }

		void force_wake	() { state = active;	on_wake	();	}
		void force_sleep() { state = inactive;	on_sleep(); }

		virtual void	on_load		() { force_wake(); }
		virtual void	on_wake		() {}
		virtual void	on_sleep	() {}

	public:
		virtual ~status() { preservatives.clear(); if (loader) { while (!loader->joinable()); loader->join(); delete loader; } }//{ _ASSERT_EXPR(loader == nullptr, L"Status deleted improperly"); }

		void try_end_load() {
			if (!loader) return;
			if (loader->joinable() && loaded.load(std::memory_order_acquire)) {
				loader->join(); 
				on_load(); 
				delete loader; 
				loader = nullptr;
			}
		}

		inline virtual void tick(float elapsed_time) { 
			timer += elapsed_time; 
			switch (state) { 
			case unloaded:	if (loader) { try_end_load(); }
							else if (!loader) {loader = new std::thread(&status::coinit, this); }		break;
			case active:	update(elapsed_time);														break;
			default:		idle(elapsed_time);															break;
			}
		};

		void			wake	()	{ if (state != inactive)	{ return; } force_wake	();	}
		void			sleep	()	{ if (state != active)		{ return; } force_sleep	();	}
		virtual void	kill	()	{} // Ubruptly end the task because I'm about to delete you

		virtual void update	(float elapsed_time) { finish(); }
		virtual void idle	(float elapsed_time) {}

		activity	report		() const { return state; }
		float		get_time	() const { return timer; }

		void preserve		(void* p)	{ preservatives.insert(p); }
		void unpreserve		(void* p)	{ preservatives.erase(p); }
		bool is_preserved	() const	{ return preservatives.size() > 0; }

		task_id get_id	() const		{ return id; }
		bool	init_id	(task_id tid)	{ if (!id) { id = tid; return true; } else { return false; } }
	};

}