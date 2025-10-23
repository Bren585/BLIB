#include "pch.h"
#include "viewer.h"

namespace BLIB {

	namespace viewer {
		
		std::unique_ptr<camera> main_camera;
		camera* active_camera = nullptr;

		void init() {
			main_camera = std::make_unique<perspective_camera>();
			set_light(LIGHT_DEFAULT);
			set_light_color(LIGHT_COLOR_DEFAULT);
			set_light_intensity(LIGHT_INTENSITY_DEFAULT);
			set_ambient_color(AMBIENT_COLOR_DEFAULT);
			set_ambient_intensity(AMBIENT_INTENSITY_DEFAULT);
			set_main();
		}

		void uninit() { }

		float3	light;
		color	light_color;
		float	light_intensity;

		float3	get_light			()				{ return light; }
		color	get_light_color		()				{ return light_color; }
		float	get_light_intensity	()				{ return light_intensity; }

		void	set_light			(float3 pos)	{ light  = pos;	}
		void	set_light_color		(color c)		{ light_color = c; }
		void	set_light_intensity (float v)		{ light_intensity = clamp(0, v, LIGHT_INTENSITY_MAX); }
		
		void	add_light			(float3 pos)	{ light += pos;	}
		void	add_light_color		(color c)		{ set_light_color(light_color.rgb() + c.rgb()); }
		void	add_light_intensity (float v)		{ set_light_intensity(light_intensity + v); }

		color ambient_color;
		float ambient_intensity;

		color	get_ambient_color		()			{ return ambient_color; }
		float	get_ambient_intensity	()			{ return ambient_intensity; }

		void	set_ambient_color		(color c)	{ ambient_color = c; }
		void	set_ambient_intensity	(float v)	{ ambient_intensity = clamp(0, v, AMBIENT_INTENSITY_MAX); }

		void	add_ambient_color		(color c)	{ set_ambient_color(ambient_color.rgb() + c.rgb()); }
		void	add_ambient_intensity	(float v)	{ set_ambient_intensity(ambient_intensity + v); }

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
		data.light_color = { viewer::get_light_color().rgb(), viewer::get_light_intensity() };
		data.ambient_color = { viewer::get_ambient_color().rgb(), viewer::get_ambient_intensity() };
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