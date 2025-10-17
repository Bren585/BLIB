#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <wrl.h>

class device {
private:
	Microsoft::WRL::ComPtr<ID3D11Device>				device_ptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>			immediate_context;

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D11Debug>					debug_interface;
	Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation>	annotation;
	Microsoft::WRL::ComPtr<ID3D11InfoQueue>				info_queue;
#endif

private:
	device() = default;
	~device() = default;

	static device& instance() { static device _instance; return _instance; }

	static void load_debug() {
#ifdef _DEBUG
		if (!instance().device_ptr) return;
		if (!instance().debug_interface) {
			HRESULT hr{ S_OK };

			hr = get()->QueryInterface(IID_ID3D11Debug, (void**)(instance().debug_interface.GetAddressOf())); VERIFY;
			hr = get()->QueryInterface(IID_ID3D11InfoQueue, (void**)(instance().info_queue.GetAddressOf())); VERIFY;

			instance().info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
			instance().info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
			instance().info_queue->AddStorageFilterEntries(nullptr);
		}
#endif
	}

public:
	device(device const&) = delete;
	device(device const&&) = delete;
	device operator=(device const&) = delete;
	device operator=(device const&&) = delete;

	static ID3D11Device*			get()				{ return instance().device_ptr.Get(); }
	static ID3D11Device**			get_address()		{ return instance().device_ptr.GetAddressOf(); }

	static ID3D11DeviceContext*		context()			{ return instance().immediate_context.Get(); }
	static ID3D11DeviceContext**	context_address()	{ return instance().immediate_context.GetAddressOf(); }

	static ID3D11Debug* debug() {
#ifdef _DEBUG
		load_debug();
		return instance().debug_interface.Get();
#else 
		return nullptr;
#endif // _DEBUG
	}

	static void report_live() {
#ifdef _DEBUG
		debug()->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
	}

	static void init_annotation() {
#ifdef _DEBUG 
		context()->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)instance().annotation.GetAddressOf());
#endif
	}

	static void begin_note(const wchar_t* note) {
#ifdef _DEBUG 
		if (instance().annotation && instance().annotation->GetStatus()) {
			instance().annotation->BeginEvent(note);
		}
#endif
	}

	static void end_note() {
#ifdef _DEBUG 
		if (instance().annotation && instance().annotation->GetStatus()) {
			instance().annotation->EndEvent();
		}
#endif
	}

};

class annotator {
public:
	annotator(const wchar_t* note) { device::begin_note(note); }
	~annotator() { device::end_note(); }
};

#define ANNOTATE(msg) annotator _(msg);