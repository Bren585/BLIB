#pragma once
#include <d3d11.h>
#include <wrl.h>
#include "string.h"
#include "math.h"
#include "shader.h"

#define SPRITE_VS DEFAULT_FLAT

namespace BLIB {

	void make_quad_buffer(ID3D11Buffer** out);

	class sprite {
	public:
		struct vertex {
			float3 position;
			float2 texcoord;
		};

		struct constants {
			color color;
		};

		enum flags {
			no_flags		= 0,
			full_filename	= 1 << 0,
			make_vbuffer	= 1 << 1,
			make_cbuffer	= 1 << 2,
			load_texture	= 1 << 3,
			load_shaders	= 1 << 4,

			make_buffers	= make_vbuffer | make_cbuffer,

			clone_flags		= make_buffers,
			dummy_flags		= load_shaders	| make_buffers,
			canvas_flags	= load_shaders	| make_buffers,
			batch_flags		= load_shaders	| load_texture		| make_cbuffer,
			font_flags		= batch_flags	| full_filename,
			default_flags	= load_shaders	| make_buffers		| load_texture
		};

	private:
		static bool		y_invert;
		static float2	viewport;
		static string	filepath;

		void create_vertex_buffer();
		void create_constant_buffer();

	protected:
		string												vs_cso;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				constant_buffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shader_resource_view	= nullptr;
		D3D11_TEXTURE2D_DESC								texture2d_desc{};

		bool create_vertices(vertex* vertex_out, float2 pos, float2 size, float2 tpos, float2 tsize, float angle, float2 center);

	public:
		sprite(flags flags, const string& filename = "");
		sprite(const string& filename) : sprite(default_flags, filename) {}
		sprite(color c, float2 size);
		sprite(const sprite&) = delete;
		sprite(sprite&&) = default;
		~sprite() {}

		static void load_shader(string vs = SPRITE_VS);
		void		load_file(const string& filename, bool full_filepath);

		sprite* clone() const;

		ID3D11ShaderResourceView**								get_release_SRV	()			{ return shader_resource_view.Get() ? shader_resource_view.ReleaseAndGetAddressOf() : shader_resource_view.GetAddressOf(); }
		ID3D11ShaderResourceView**								get_SRV			()			{ return shader_resource_view.Get() ? shader_resource_view.GetAddressOf()			: nullptr; }
		const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& peek_SRV		() const	{ return shader_resource_view; }

		static vertex_shader get_default_vs() { return vertex_shader{ SPRITE_VS }; }

		static void		set_y_invert(bool invert)	{ y_invert = invert; }
		static void		set_viewport(float2 size)	{ viewport = size; }
		static void		set_filepath(string path)	{ filepath = path; }
		
		static bool		get_y_invert()				{ return y_invert; }
		static float2	get_viewport()				{ return viewport; }
		static string	get_filepath()				{ return filepath; }

		float2 get_size	()				const		{ return { (float)texture2d_desc.Width, (float)texture2d_desc.Height }; }
		void   resize	(float2 size)				{ texture2d_desc.Width = (UINT)size.x; texture2d_desc.Height = (UINT)size.y; }

		virtual void	render(float2 pos,			float2 size,		float2 tpos = 0,			float2 tsize = 0,			float angle = 0, float2 center = C_CC, color color = {1, 1, 1});
		void			render(float dx, float dy,	float dw, float dh, float sx = 0, float sy = 0, float sw = 0, float sh = 0, float angle = 0, float2 center = C_CC, float r = 1, float g = 1, float b = 1, float a = 1) { render({dx, dy}, {dw, dh}, {sx, sy}, {sw, sh}, angle, center, {r, g, b, a}); }
	};

}