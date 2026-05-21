#pragma once
#include "math.h"
#include "window.h"
#include <d3d11.h>
#include <wrl.h>
#include "lights.h"

#define SHADOW_MAP_SIZE					2048
#define MAX_SHADOWS						8

namespace BLIB {

	namespace generic { class scene; }

	namespace lighting {

		struct lighting_constants {
			int			light_count;
			float3		skylight_direction;
			float4		skylight_color;
			float4x4	skylight_view_proj;
			float4		ambient_color;
		};

		void init();
		void uninit();
		void bind_lights (const generic::scene* geometry, const environment_lights* scene_lights, const std::vector<light>* lights);
		inline void bind_lights (const generic::scene* geometry) { bind_lights(geometry, nullptr, nullptr); }

#ifdef _DEBUG
		ID3D11ShaderResourceView* imgui_get_shadow_slice(int slice);
#endif

	}

}