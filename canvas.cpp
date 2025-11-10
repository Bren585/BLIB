#include "pch.h"
#include "canvas.h"
#include "render_lock.h"
#include "text.h"

using namespace BLIB;

canvas::canvas(float2 size) : view(size) {
	set_sprite(new sprite(sprite::canvas_flags));
	view.copy_SRV_to(peek_sprite()->get_release_SRV());
	peek_sprite()->resize(size);
	object::size = size;
}

void canvas::resize(float2 size) {
	view.resize(size);
	view.copy_SRV_to(peek_sprite()->get_release_SRV());
	peek_sprite()->resize(size);
	object::size = size;
}

void canvas::clear() const { RENDER_LOCK; view.clear(background); }

void canvas::draw(renderable* r, render_settings rs) const {
	if (!r) return;
	RENDER_LOCK;
	focus();
	r->render(rs);
}

float canvas::type(string s, float2 pos, float2 size, font_name font, color color, float2 align) const {
	focus();
	return text::out(s, pos, size, font, color, align);
}

void canvas::snapshot_to_sprite(sprite* target) const {
	HRESULT hr;

	ID3D11Resource* resource;
	view.get_SRV_resource(&resource);

	ID3D11Texture2D* texture;
	hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);
	resource->Release(); VERIFY;

	D3D11_TEXTURE2D_DESC desc = {};
	texture->GetDesc(&desc);
	desc.MiscFlags = 0;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* out_texture;
	hr = device::get()->CreateTexture2D(&desc, nullptr, &out_texture); VERIFY;

	device::context()->CopyResource(out_texture, texture);
	texture->Release();

	ID3D11ShaderResourceView* out_srv;
	hr = device::get()->CreateShaderResourceView(out_texture, nullptr, &out_srv); VERIFY;
	out_texture->Release();

	ID3D11ShaderResourceView** target_srv = target->get_release_SRV();
	*target_srv = out_srv;

	target->resize({ (float)desc.Width, (float)desc.Height });
}