#pragma once
#include "verify.h"
#include <thread>
#include <atomic>
#include <windows.h>

namespace BLIB {

	class status {
	public:
		enum activity {
			inactive,
			active,
			complete
		};

	private:
		std::thread*		loader = nullptr;
		std::atomic<bool>	loaded { false };

		bool _preserve = false;

	protected:
		activity	state	= inactive;
		float		timer	= 0;

		void coinit() { srand(static_cast<unsigned int>(time(nullptr))); HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED); VERIFY; init(); CoUninitialize(); loaded.store(true, std::memory_order_release); }
		void finish() { state = complete; }

	public:
		virtual ~status() { if (loader) { while(!loader->joinable()); loader->join(); delete loader; } }//{ _ASSERT_EXPR(loader == nullptr, L"Status deleted improperly"); }

		inline virtual void tick(float elapsed_time) { 
			timer += elapsed_time; 
			switch (state) { 
			case inactive:	if (loader) { 
								if (loader->joinable() && loaded.load(std::memory_order_acquire)) { 
									loader->join(); state = active; delete loader; loader = nullptr; 
								} 
							}
							else if (!loader) {loader = new std::thread(&status::coinit, this); }		break;
			case active:	update(elapsed_time);														break;
			default:		idle(elapsed_time);															break;
			}
		};

		virtual void init() { }
		virtual void update(float elapsed_time) { state = complete; }
		virtual void idle(float elapsed_time) { }
		virtual void wake() {}
		virtual void kill() {} // Ubruptly end the task
		activity report() const { return state; }

		void preserve()				{ _preserve = true; }
		void unpreserve()			{ _preserve = false; }
		bool is_preserved() const	{ return _preserve; }
	};

}