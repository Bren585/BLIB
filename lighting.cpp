#include "pch.h"
#include "lighting.h"
#include "scene.h"
#include "camera.h"

namespace BLIB {

	/* environmental_lights **********************************************************************************************/

	// Skylight

	float3	environment_lights::get_skylight_direction	() const			{ return skylight_direction;	}
	color	environment_lights::get_skylight_color		() const			{ return skylight_color;		}
	float	environment_lights::get_skylight_intensity	() const			{ return skylight_intensity;	}

	void	environment_lights::set_skylight_direction	(const float3& n)	{ skylight_direction = n.norm();							}
	void	environment_lights::set_skylight_color		(const color& c)	{ skylight_color = c;										}
	void	environment_lights::set_skylight_intensity	(float v)			{ skylight_intensity = clamp(0, v, SKYLIGHT_INTENSITY_MAX);	}
	
	void environment_lights::load_skylight_view_proj(const camera* cam, float4x4& out) const {
		float3 center;
		float3 corner[8];
		float depth;
		{
			UINT index = 0;
			float min_d = FLT_MAX, max_d = -FLT_MAX;
			float min_w = FLT_MAX, max_w = -FLT_MAX;
			float min_h = FLT_MAX, max_h = -FLT_MAX;
			for (float x = -1; x < 2; x += 2) {
				for (float y = -1; y < 2; y += 2) {
					for (float z = 0; z < 2; z += 1) {
						float4 world = mul({ x, y, z, 1 }, cam->get_inverse_view_projection());
						corner[index] = world.xyz() / world.w;
						center += corner[index];
						min_d = minim(min_d, dot(corner[index], skylight_direction));
						max_d = maxim(max_d, dot(corner[index], skylight_direction));
						index++;
					}
				}
			}
			depth	= (max_d - min_d) * 0.5f + EPS;
		}
		center /= 8;
		float3 up = (fabsf(dot(float3(0, 1, 0), skylight_direction)) < 0.99f ? float3(0, 1, 0) : float3(1, 0, 0));
		matrix skylight_view = DirectX::XMMatrixLookAtLH((xmvector)(center - (skylight_direction * depth)), (xmvector)center, (xmvector)up);
		
		float3 min = float3{ FLT_MAX }, max = float3{ -FLT_MAX };
		for (int i = 0; i < 8; i++) {
			float4 raw = mul(float4(corner[i], 1), skylight_view);
			float3 light_corner = raw.xyz() / raw.w;
			min = minimize(min, light_corner);
			max = maximize(max, light_corner);
		}
		float width		= max.x - min.x;
		float height	= max.y - min.y;

		float2 per_texel = float2(width, height) / SHADOW_MAP_SIZE;

		float3 shadow_origin = (mul(float4(center, 1), skylight_view).xyz() / float3(per_texel, 1));
		float2 rounded = (shadow_origin.xy() + float2{ 0.5f }).floor();
		float2 offset = (rounded - shadow_origin.xy()) * per_texel;

		skylight_view.r[3] = DirectX::XMVectorAdd(skylight_view.r[3], (xmvector)float4(offset, 0, 0));

		matrix skylight_proj = DirectX::XMMatrixOrthographicLH(width, height, min.z - EPS, max.z + EPS);

		DirectX::XMStoreFloat4x4(&out, skylight_view * skylight_proj);
	}

	// Ambient

	color	environment_lights::get_ambient_color			() const			{ return ambient_color; }
	float	environment_lights::get_ambient_intensity		() const			{ return ambient_intensity; }

	void	environment_lights::set_ambient_color			(const color& c)	{ ambient_color = c; }
	void	environment_lights::set_ambient_intensity		(float v)			{ ambient_intensity = clamp(0, v, AMBIENT_INTENSITY_MAX); }

	namespace lighting {

		lighting_constants									constant_buffer_data;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				constant_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				structured_buffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	structured_buffer_SRV;

		struct packed_light {
			float3		position;
			float3		direction;
			float3		color;
			float		intensity		= 0;
			float		spread			= 0;
			float		fade			= 0;
			UINT		shadow_index	= 0;
			float4x4	view_proj		= matrix_id;

			packed_light() = default;
			packed_light(const light& l) : position(l.position), direction(l.direction), color(l.tint.rgb()), intensity(l.intensity), spread(l.spread), fade(l.fade), view_proj(l.get_view_proj()) {}
		};

		std::vector<packed_light> packed_lights;

		Microsoft::WRL::ComPtr<ID3D11Buffer>						shadow_constant_buffer;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>						shadow_map_array{nullptr};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>			shadow_SRV;
		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> shadow_stencil;
		D3D11_VIEWPORT												shadow_viewport{};

#ifdef _DEBUG
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> shadow_slices;
		ID3D11ShaderResourceView* imgui_get_shadow_slice(int slice) { return shadow_slices.at(slice).Get(); }
#endif

		void init_shadow_map_array() {
			HRESULT hr{S_OK};

			/* constant buffer */ {
				D3D11_BUFFER_DESC desc{};
				desc.ByteWidth				= sizeof(float4x4);
				desc.Usage					= D3D11_USAGE_DEFAULT;
				desc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
				desc.CPUAccessFlags			= 0;
				desc.MiscFlags				= 0;
				desc.StructureByteStride	= 0;

				hr = device::get()->CreateBuffer(&desc, nullptr, shadow_constant_buffer.GetAddressOf()); VERIFY;
			}

			/* texture array */ {
				D3D11_TEXTURE2D_DESC desc{};
				desc.Width = desc.Height	= SHADOW_MAP_SIZE;
				desc.MipLevels				= 1;
				desc.ArraySize				= MAX_SHADOWS;
				desc.Format					= DXGI_FORMAT_R32_TYPELESS;
				desc.SampleDesc.Count		= 1;
				desc.Usage					= D3D11_USAGE_DEFAULT;
				desc.BindFlags				= D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags			= 0;
				desc.MiscFlags				= 0;

				hr = device::get()->CreateTexture2D(&desc, nullptr, shadow_map_array.GetAddressOf()); VERIFY;
			}

			/* srv */ {
				D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
				desc.Format							= DXGI_FORMAT_R32_FLOAT;
				desc.ViewDimension					= D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = 0;
				desc.Texture2DArray.MipLevels		= 1;
				desc.Texture2DArray.FirstArraySlice = 0;
				desc.Texture2DArray.ArraySize		= MAX_SHADOWS;

				hr = device::get()->CreateShaderResourceView(shadow_map_array.Get(), &desc, shadow_SRV.GetAddressOf()); VERIFY;
			}

#ifdef _DEBUG
			/* slice srvs */ {
				D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
				desc.Format = DXGI_FORMAT_R32_FLOAT;
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = 0;
				desc.Texture2DArray.MipLevels = 1;
				desc.Texture2DArray.ArraySize = 1;

				for (int i = 0; i < MAX_SHADOWS; i++) {
					desc.Texture2DArray.FirstArraySlice = i;
					auto& srv = shadow_slices.emplace_back();
					hr = device::get()->CreateShaderResourceView(shadow_map_array.Get(), &desc, srv.GetAddressOf()); VERIFY;
				}
			}
#endif

			/* depth stencil */ {
				shadow_stencil.resize(MAX_SHADOWS);
				D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
				desc.Format = DXGI_FORMAT_D32_FLOAT;
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = 0;
				desc.Texture2DArray.ArraySize = 1;
				for (UINT i = 0; i < MAX_SHADOWS; i++) {
					desc.Texture2DArray.FirstArraySlice = i;
					hr = device::get()->CreateDepthStencilView(shadow_map_array.Get(), &desc, shadow_stencil[i].GetAddressOf());
				}
			}

			/* Structured Buffer */ {
				D3D11_BUFFER_DESC desc{};
				desc.ByteWidth = static_cast<uint>(MAX_LIGHTS * sizeof(packed_light));
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
				desc.StructureByteStride = sizeof(packed_light);
				hr = device::get()->CreateBuffer(&desc, nullptr, structured_buffer.GetAddressOf()); VERIFY;
			}

			/* SRV */ {
				D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
				desc.Format = DXGI_FORMAT_UNKNOWN;
				desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				desc.Buffer.FirstElement = 0;
				desc.Buffer.NumElements = MAX_LIGHTS;
				hr = device::get()->CreateShaderResourceView(structured_buffer.Get(), &desc, structured_buffer_SRV.GetAddressOf()); VERIFY;
			}

			shadow_viewport.Width = shadow_viewport.Height = SHADOW_MAP_SIZE;
			shadow_viewport.MinDepth = 0.0f;
			shadow_viewport.MaxDepth = 1.0f;

			D3D11_INPUT_ELEMENT_DESC input_element_desc[1] = {
				{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			};

			shader::load_vs("shadow", input_element_desc, 1);
			shader::load_ps("shadow");

		}

		void make_shadows(const generic::scene* geometry, const std::vector<light>* lights) {
			// Cache pipeline state
			UINT						cached_viewport_count{ 1 };
			D3D11_VIEWPORT				cached_viewport;
			UINT						cached_render_target_view_count{ MAX_VIEWS };
			ID3D11RenderTargetView*		cached_render_target_view[MAX_VIEWS] = {};
			ID3D11DepthStencilView*		cached_depth_stencil_view{ nullptr };
			device::context()->RSGetViewports(&cached_viewport_count, &cached_viewport);
			device::context()->OMGetRenderTargets(cached_render_target_view_count, cached_render_target_view, &cached_depth_stencil_view);

			device::context()->RSSetViewports(1, &shadow_viewport);
			UINT shadow_count = 0;
			// No Shadow
			device::context()->ClearDepthStencilView(shadow_stencil[shadow_count++].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

			render_settings rs{ vertex_shader("shadow"), pixel_shader("shadow"), geometry_shader(NULL_SHADER) };

			/* Skylight */ {
				annotate("skylight shadows");
				device::context()->UpdateSubresource(shadow_constant_buffer.Get(), 0, 0, &constant_buffer_data.skylight_view_proj, 0, 0);
				device::context()->VSSetConstantBuffers(1, 1, shadow_constant_buffer.GetAddressOf());
				auto dsv = shadow_stencil[shadow_count++].Get();
				device::context()->OMSetRenderTargets(0, nullptr, dsv);
				device::context()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
				geometry->draw(rs);
			}

			// All other lights
			for (int i = 0; i < lights->size(); i++) {
				annotate(string("source ", i, " shadows"));
				const light& light = lights->at(i);
				if (shadow_count >= MAX_SHADOWS) break;
				if (!light.casts_shadows()) continue;
				packed_lights.at(i).shadow_index = shadow_count;
				device::context()->UpdateSubresource(shadow_constant_buffer.Get(), 0, 0, &light.get_view_proj(), 0, 0);
				device::context()->VSSetConstantBuffers(1, 1, shadow_constant_buffer.GetAddressOf());
				auto dsv = shadow_stencil[shadow_count].Get();
				device::context()->OMSetRenderTargets(0, nullptr, dsv);
				device::context()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
				geometry->draw(rs);
				shadow_count++;
			}

			// Uncache
			device::context()->RSSetViewports(cached_viewport_count, &cached_viewport);
			device::context()->OMSetRenderTargets(cached_render_target_view_count, cached_render_target_view, cached_depth_stencil_view);
			for (int i = 0; i < MAX_VIEWS; i++) { if (cached_render_target_view[i]) cached_render_target_view[i]->Release(); }
			if (cached_depth_stencil_view) cached_depth_stencil_view->Release();

			device::context()->PSSetShaderResources(6, 1, shadow_SRV.GetAddressOf());
		}

		void pack_lights(const std::vector<light>* lights = nullptr) {
			packed_lights.clear();
			if (!lights) { packed_lights.push_back(packed_light()); return; }
			for (auto& light : *lights) { packed_lights.push_back(light); }
			constant_buffer_data.light_count = static_cast<int>(packed_lights.size());
			if (packed_lights.empty()) { packed_lights.push_back(packed_light()); }
		}

		void update_buffers() {
			device::context()->UpdateSubresource(constant_buffer.Get(), 0, 0, &constant_buffer_data, 0, 0);
			device::context()->PSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

			_ASSERT_EXPR(constant_buffer_data.light_count < MAX_LIGHT_COUNT, "Too Many Lights");

			device::context()->UpdateSubresource(structured_buffer.Get(), 0, 0, packed_lights.data(), 0, 0);
			device::context()->PSSetShaderResources(5, 1, structured_buffer_SRV.GetAddressOf());			
		}

		void bind_lights(const generic::scene* geometry, const environment_lights* scene_lights, const std::vector<light>* lights) {
			if (!geometry) return;
			const environment_lights* geometry_lights_ptr	= scene_lights	? scene_lights	: &geometry->get_scene_lights();
			const std::vector<light>* lights_ptr			= lights		? lights		: &geometry->get_lights();

			// Skylight
			constant_buffer_data.skylight_direction =   geometry_lights_ptr->get_skylight_direction();
			constant_buffer_data.skylight_color		= { geometry_lights_ptr->get_skylight_color().rgb(),	geometry_lights_ptr->get_skylight_intensity() };
			geometry_lights_ptr->load_skylight_view_proj(geometry->get_camera(), constant_buffer_data.skylight_view_proj);

			// Ambient
			constant_buffer_data.ambient_color		= { geometry_lights_ptr->get_ambient_color().rgb(),	geometry_lights_ptr->get_ambient_intensity() };

			// Lights
			pack_lights(lights_ptr);
			make_shadows(geometry, lights_ptr);
			update_buffers();
		}

		void init() {
			HRESULT hr{ S_OK };
			/* Constant Buffer */ {
				D3D11_BUFFER_DESC buffer_desc{};
				buffer_desc.ByteWidth			= sizeof(lighting_constants);
				buffer_desc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
				buffer_desc.Usage				= D3D11_USAGE_DEFAULT;
				buffer_desc.CPUAccessFlags		= 0;
				buffer_desc.MiscFlags			= 0;
				buffer_desc.StructureByteStride = 0;
				hr = device::get()->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf()); VERIFY;
			}

			//pack_lights();
			//update_buffers();

			init_shadow_map_array();
		}

	}
}