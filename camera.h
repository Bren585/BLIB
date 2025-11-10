#pragma once
#include "math.h"
#include "window.h"
#include <d3d11.h>

#define P_EYE_DEFAULT		{ 0.0f, 0.0f, -10.0f }
#define P_FOCUS_DEFAULT		{ 0.0f, 0.0f, 0.0f }
#define P_CLIP_DEFAULT		{ 0.1f, 100.0f }
#define P_FOV_DEFAULT		DirectX::XMConvertToRadians(30.0f)

#define O_EYE_DEFAULT		{ 0.0f, 0.0f, -1.0f }
#define O_FOCUS_DEFAULT		{ 0.0f, 0.0f, 0.0f }
#define O_CLIP_DEFAULT		{ 0.00f, 1.0f }

namespace BLIB {
	class camera {
		struct constants {
			//float4x4 view;
			float4x4 view_projection;
			float4x4 inverse_view_projection;
			//float4x4 inverse_view;
			//float4x4 inverse_projection;
			float3 camera_position;
			float far_z;
		};
	private:
		mutable constants data;
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;

		float2 clip_range;
		float3 eye;
		float3 focus;
		float3 up;

		void update() const { if (needs_update) { _update(); needs_update = false; } }

	protected:
		mutable matrix V;
		mutable matrix P;
		mutable matrix VP;
		mutable matrix IV;
		mutable matrix IP;
		mutable matrix IVP;

		mutable bool needs_update = true;
		virtual void _update() const = 0;

	public:
		camera(float2 clip_range, float3 eye, float3 focus, float3 up = {0, 1, 0});
		virtual ~camera() {}

		float3	get_eye()		const { return eye;				}
		float3	get_focus()		const { return focus;			}
		float3	get_up()		const { return up;				}
		float	get_near()		const { return clip_range.x;	}
		float	get_far()		const { return clip_range.y;	}
		float3	get_facing()	const { return focus - eye;		}

		void	set_eye		(float3 e)	{ eye = e;			needs_update = true; }
		void	set_focus	(float3 f)	{ focus = f;		needs_update = true; }
		void	set_up		(float3 u)	{ up = u;			needs_update = true; }
		void	set_near	(float d)	{ clip_range.x = d;	needs_update = true; }
		void	set_far		(float d)	{ clip_range.y = d;	needs_update = true; }

		const matrix& get_view()					const { update(); return V;		}
		const matrix& get_projection()				const { update(); return P;		}
		const matrix& get_view_projection()			const { update(); return VP;	}
		const matrix& get_inverse_view()			const { update(); return IV;	}
		const matrix& get_inverse_projection()		const { update(); return IP;	}
		const matrix& get_inverse_view_projection()	const { update(); return IVP;	}

		void bind() const;
	};

	class perspective_camera : public camera {
	private:
		float aspect_ratio;
		float fov;

		void _update() const override;
	public:
		perspective_camera(float2 viewport = window::size(), float3 eye = P_EYE_DEFAULT, float3 focus = P_FOCUS_DEFAULT, float fov = P_FOV_DEFAULT, float2 clip_range = P_CLIP_DEFAULT) :
			camera(clip_range, eye, focus), aspect_ratio(viewport.x / viewport.y), fov(fov) {}
		~perspective_camera() {}

		float get_aspect()	const { return aspect_ratio;	}
		float get_fov()		const { return fov;				}

		void set_aspect	(float2 viewport)	{ aspect_ratio = viewport.x / viewport.y;	needs_update = true; }
		void set_fov	(float f)			{ fov = f;									needs_update = true; }

	};

	class orthographic_camera : public camera {
	private:
		float2 viewport;

		void _update() const override;
	public:
		orthographic_camera(float2 viewport = window::size(), float3 eye = O_EYE_DEFAULT, float3 focus = O_FOCUS_DEFAULT, float2 clip_range = O_CLIP_DEFAULT) :
			camera(clip_range, eye, focus), viewport(viewport) { }
		~orthographic_camera() {}

		float2 get_viewport() const { return viewport; }

		void set_viewport(float2 v) { viewport = v; needs_update = true; }
	};
}