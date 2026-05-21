#pragma once
#include "math.h"
#include "lighting_constants.h"

#define SKYLIGHT_DEFAULT				{ -0.5f, -0.5f, -0.5f }
#define SKYLIGHT_COLOR_DEFAULT			WHITE
#define SKYLIGHT_INTENSITY_DEFAULT		3
#define SKYLIGHT_INTENSITY_MAX			10

#define AMBIENT_COLOR_DEFAULT			WHITE
#define AMBIENT_INTENSITY_DEFAULT		1
#define AMBIENT_INTENSITY_MAX			10

#define LIGHT_INTENSITY_DEFAULT			2
#define LIGHT_INTENSITY_MAX				10

namespace BLIB {

	namespace lighting	{ struct packed_light; }
	
	class camera;

	class light {
		friend struct lighting::packed_light;
	private:
		float3 position		= { 0, 0, 0 };
		float3 direction	= { 1, 0, 0 };
		color tint			= WHITE;
		float intensity		= LIGHT_INTENSITY_DEFAULT;
		float spread		= 0; // How big the spotlight is, from 0-1, where 1 is a point light, 0.25 is a big circle, 0.1 is a small circle, and 0 is no light.
		float fade			= 0; // How fast the light falls off as it approaches the edge of the circle, where 0 is no fade (perfect circle) and 1 is a pinpoint.
	
		bool active			= true;
		bool shadows		= true;
		mutable bool dirty	= true;

		mutable float4x4 view_proj;
	
	public:
		light() = default;
		light(const light&) = default;

		void set_position	(float3 v	) { position	= v; dirty = true;	}
		void set_direction	(float3 v	) { direction	= v; dirty = true;	}
		void set_tint		(color v	) { tint		= v;				}
		void set_intensity	(float v	) { intensity	= v;				}
		void set_spread		(float v	) { spread		= v; dirty = true;	}
		void set_fade		(float v	) { fade		= v;				}

		float3	get_position	() const { return position;		}
		float3	get_direction	() const { return direction;	}
		color	get_tint		() const { return tint;			}
		float	get_intensity	() const { return intensity;	}
		float	get_spread		() const { return spread;		}
		float	get_fade		() const { return fade;			}

		const float4x4& get_view_proj() const {
			if (dirty) {
				DirectX::XMStoreFloat4x4(&view_proj,
					DirectX::XMMatrixLookAtLH((xmvector)position, (xmvector)(position + direction), VECTOR_UP) *
					DirectX::XMMatrixPerspectiveFovLH(maxim(spread * PI, 0.01f), 1, LIGHT_NEAR, LIGHT_FAR)
				);
			}
			return view_proj;
		}

		bool casts_shadows	() const	{ return shadows;	}
		void enable_shadows	()			{ shadows = true;	}
		void disable_shadows()			{ shadows = false;	}

		bool is_active	() const	{ return active;	}
		void enable		()			{ active = true;	}
		void disable	()			{ active = false;	}
		void toggle		()			{ active = !active;	}
	};

	class environment_lights {
	private: // Skylight
		float3	skylight_direction		= SKYLIGHT_DEFAULT;
		color	skylight_color			= SKYLIGHT_COLOR_DEFAULT;
		float	skylight_intensity		= SKYLIGHT_INTENSITY_DEFAULT;
		matrix	skylight_view_proj		= MATRIX_ID;
	public:
		float3	get_skylight_direction	() const;
		color	get_skylight_color		() const;
		float	get_skylight_intensity	() const;

		void	set_skylight_direction	(const float3& pos);
		void	set_skylight_color		(const color& c);
		void	set_skylight_intensity	(float v);

		void	load_skylight_view_proj	(const camera* cam, float4x4& out) const;

	private: // Ambient
		color	ambient_color			= AMBIENT_COLOR_DEFAULT;
		float	ambient_intensity		= AMBIENT_INTENSITY_DEFAULT;
	public:
		color	get_ambient_color		() const;
		float	get_ambient_intensity	() const;

		void	set_ambient_color		(const color& c);
		void	set_ambient_intensity	(float v);

	};

}