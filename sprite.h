#pragma once
#include <d3d11.h>
#include <wrl.h>

#include "string.h"
#include "math.h"

namespace BLIB {

	class sprite {
	public:
		struct vertex {
			float3 position;
			float4 color;
			float2 texcoord;
		};

		enum flags {
			no_flags		= 0,
			full_filename	= 1 << 0,
			make_buffer		= 1 << 1,
			load_texture	= 1 << 2,

			batch_flags		= load_texture,
			dummy_flags		= make_buffer,
			font_flags		= full_filename | load_texture,
			default_flags	= make_buffer	| load_texture
		};

	private:
		static bool		y_invert;
		static float2	viewport;
		static string	filepath;

		void create_buffer();

	protected:
		string												vs_cso					= "sprite";
		Microsoft::WRL::ComPtr<ID3D11Buffer>				vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shader_resource_view;
		D3D11_TEXTURE2D_DESC								texture2d_desc;

	public:
		sprite(flags flags, const string& filename = "");
		sprite(const string& filename) : sprite(default_flags, filename) {}
		sprite(color c, float2 size);
		sprite(const sprite&) = delete;
		~sprite() {}

		sprite* clone() const;

		ID3D11ShaderResourceView**								SRV_copy_dest	()			{ return shader_resource_view.Get() ? shader_resource_view.ReleaseAndGetAddressOf() : shader_resource_view.GetAddressOf(); }
		const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& peek_SRV		() const	{ return shader_resource_view; }

		static void		set_y_invert(bool invert)	{ y_invert = invert; }
		static void		set_viewport(float2 size)	{ viewport = size; }
		static void		set_filepath(string path)	{ filepath = path; }
		
		static bool		get_y_invert()				{ return y_invert; }
		static float2	get_viewport()				{ return viewport; }
		static string	get_filepath()				{ return filepath; }

		float2 get_size	()				const		{ return { (float)texture2d_desc.Width, (float)texture2d_desc.Height }; }
		void   resize	(float2 size)				{ texture2d_desc.Width = (UINT)size.x; texture2d_desc.Height = (UINT)size.y; }

		bool create_vertices(vertex* vertex_out, float2 pos, float2 size, float2 tpos, float2 tsize, float angle, float2 center, color color);

		virtual void	render(float2 pos,			float2 size,		float2 tpos = 0,			float2 tsize = 0,			float angle = 0, float2 center = C_CC, color color = {1, 1, 1});
		void			render(float dx, float dy,	float dw, float dh, float sx = 0, float sy = 0, float sw = 0, float sh = 0, float angle = 0, float2 center = C_CC, float r = 1, float g = 1, float b = 1, float a = 1) { render({dx, dy}, {dw, dh}, {sx, sy}, {sw, sh}, angle, center, {r, g, b, a}); }
	};

}