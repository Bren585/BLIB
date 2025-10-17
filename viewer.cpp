#include "pch.h"
#include "viewer.h"

namespace BLIB {

	namespace viewer {
		
		std::unique_ptr<camera> main_camera;
		camera* active_camera = nullptr;

		void init() {
			main_camera = std::make_unique<perspective_camera>();
			set_light(LIGHT_DEFAULT);
			set_main();
		}

		void uninit() { }

		float3	light;
		float3	get_light()				{ return light; }
		void	set_light(float3 pos)	{ light  = pos;	}
		void	add_light(float3 pos)	{ light += pos;	}


		void set_active(camera* cam) {
			active_camera = cam;
			cam->bind();
		}

		void set_main() { set_active(main_camera.get()); }

		const camera* peek_main()	{ return main_camera.get(); }
		const camera* peek_active() { return active_camera; }

		camera* get_main()			{ return main_camera.get(); }
		camera* get_active()		{ return active_camera; }
	}

	camera::camera(float2 clip_range, float3 eye, float3 focus, float3 up) 
		: clip_range(clip_range), eye(eye), focus(focus), up(up), V(MATRIX_ID), P(MATRIX_ID), VP(MATRIX_ID) {
		HRESULT hr{ S_OK };
		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth			= sizeof(viewer::scene_constants);
		buffer_desc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
		buffer_desc.CPUAccessFlags		= 0;
		buffer_desc.MiscFlags			= 0;
		buffer_desc.StructureByteStride = 0;
		hr = device::get()->CreateBuffer(&buffer_desc, nullptr, buffer.GetAddressOf()); VERIFY;
	}

	void camera::bind() const {
		data.light_direction = { viewer::get_light(), 0 };
		DirectX::XMStoreFloat4x4(&data.view_projection, get_view_projection());
		data.camera_position = { eye, 0 };
		RENDER_LOCK;
		device::context()->UpdateSubresource(buffer.Get(), 0, 0, &data, 0, 0);
		device::context()->VSSetConstantBuffers(1, 1, buffer.GetAddressOf());
		device::context()->PSSetConstantBuffers(1, 1, buffer.GetAddressOf());
	}

	void perspective_camera::_update() const {
		V = DirectX::XMMatrixLookAtLH((xmvector)get_eye(), (xmvector)get_focus(), (xmvector)get_up());
		P = DirectX::XMMatrixPerspectiveFovLH(fov, aspect_ratio, get_near(), get_far());
		VP = V * P;
	}

	void orthographic_camera::_update() const {
		V = DirectX::XMMatrixIdentity();//DirectX::XMMatrixLookAtLH((xmvector)get_eye(), (xmvector)get_focus(), (xmvector)get_up());
		P = DirectX::XMMatrixOrthographicOffCenterLH(0, viewport.x, viewport.y, 0, get_near(), get_far());
		VP = V * P;
	}

}