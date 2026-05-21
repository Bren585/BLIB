#include "pch.h"
#include "camera.h"
#include "constant_buffer_indices.h"

namespace BLIB {
	camera::camera(float2 clip_range, float3 eye, float3 focus, float3 up)
		: clip_range(clip_range), eye(eye), focus(focus), up(up), V(MATRIX_ID), P(MATRIX_ID), VP(MATRIX_ID), IVP(MATRIX_ID) {
		HRESULT hr{ S_OK };
		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth = sizeof(constants);
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		hr = device::get()->CreateBuffer(&buffer_desc, nullptr, buffer.GetAddressOf()); VERIFY;
	}

	void camera::bind() const {
		//DirectX::XMStoreFloat4x4(&data.view,					get_view()						);
		DirectX::XMStoreFloat4x4(&data.view_projection,			get_view_projection()			);
		DirectX::XMStoreFloat4x4(&data.inverse_view_projection, get_inverse_view_projection()	);
		//DirectX::XMStoreFloat4x4(&data.inverse_view,			get_inverse_view()				);
		//DirectX::XMStoreFloat4x4(&data.inverse_projection,	get_inverse_projection()		);
		data.camera_position								  = get_eye();
		data.far_z											  = get_far();

		RENDER_LOCK;
		device::context()->UpdateSubresource(buffer.Get(), 0, 0, &data, 0, 0);
		device::context()->VSSetConstantBuffers(CAMERA_CB, 1, buffer.GetAddressOf());
		device::context()->PSSetConstantBuffers(CAMERA_CB, 1, buffer.GetAddressOf());
		device::context()->GSSetConstantBuffers(CAMERA_CB, 1, buffer.GetAddressOf());
	}

	void perspective_camera::_update() const {
		V = DirectX::XMMatrixLookAtLH((xmvector)get_eye(), (xmvector)get_focus(), (xmvector)get_up());
		P = DirectX::XMMatrixPerspectiveFovLH(fov, aspect_ratio, get_near(), get_far());
		VP = V * P;
		IV = DirectX::XMMatrixInverse(nullptr, V);
		IP = DirectX::XMMatrixInverse(nullptr, P);
		IVP = DirectX::XMMatrixInverse(nullptr, VP);
	}

	void orthographic_camera::_update() const {
		V = DirectX::XMMatrixIdentity();//DirectX::XMMatrixLookAtLH((xmvector)get_eye(), (xmvector)get_focus(), (xmvector)get_up());
		P = DirectX::XMMatrixOrthographicOffCenterLH(0, viewport.x, viewport.y, 0, get_near(), get_far());
		VP = V * P;
		IV = DirectX::XMMatrixInverse(nullptr, V);
		IP = DirectX::XMMatrixInverse(nullptr, P);
		IVP = DirectX::XMMatrixInverse(nullptr, VP);
	}

}