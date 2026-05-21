#pragma once
#include "blend.h"
#include "rasterize.h"
#include "depth_stencil.h"
#include "sampler.h"
#include "shader.h"
#include "verify.h"

namespace BLIB {

	struct render_settings {
		rasterize	::state	rs = rasterize			::DEFAULT;		// rasterize state
		blend		::state	bs = blend				::DEFAULT;		// blend state
		stencil		::state	ss = stencil			::DEFAULT;		// depth stencil state
		sampler		::state ms = sampler			::DEFAULT;		// sampler state
		pixel_shader		ps = (pixel_shader)		UNDEFINED_CSO;	// pixel shader name
		vertex_shader		vs = (vertex_shader)	UNDEFINED_CSO;	// vertex shader name
		geometry_shader		gs = (geometry_shader)	NULL_SHADER;	// geometry shader name

		render_settings() = default;
		render_settings(const render_settings&) = default;
		render_settings(render_settings&&) = default;

		template <typename... Args,
			typename = std::enable_if_t<
				(std::conjunction_v<
					std::disjunction<
					std::is_same<rasterize::state,	std::decay_t<Args>>,
					std::is_same<blend::state,		std::decay_t<Args>>,
					std::is_same<stencil::state,	std::decay_t<Args>>,
					std::is_same<sampler::state,	std::decay_t<Args>>,
					std::is_same<pixel_shader,		std::decay_t<Args>>,
					std::is_same<vertex_shader,		std::decay_t<Args>>,
					std::is_same<geometry_shader,	std::decay_t<Args>>
					>...>) &&
				!(sizeof...(Args) == 1 &&
					std::conjunction_v<std::is_same<render_settings,
					std::decay_t<Args>>...>)
			> // end enable
		> // end template
		render_settings(Args&&... args) { 
			(set_state(std::forward<Args>(args)), ...);;
		}

		inline void set() const {
			rasterize	::set(rs);
			blend		::set(bs);
			stencil		::set(ss);
			sampler		::set(ms);

			_ASSERT_EXPR_A(ps != UNDEFINED_CSO, "Pixel Shader Undefined");
			shader		::set_ps(ps);

			_ASSERT_EXPR_A(vs != UNDEFINED_CSO, "Vertex Shader Undefined");
			shader		::set_vs(vs);

			_ASSERT_EXPR_A(gs != UNDEFINED_CSO, "Geometry Shader Undefined");
			shader::set_gs(gs);
		}

		inline render_settings& operator=(const render_settings& o) {
			rs = o.rs;
			bs = o.bs;
			ss = o.ss;
			ms = o.ms;
			ps = o.ps;
			vs = o.vs;
			gs = o.gs;
			return *this;
		}

		inline render_settings operator&(const render_settings& alt) const {
			return {
				( alt.rs == rasterize		:: DEFAULT					) ? rs : alt.rs,
				( alt.bs == blend			:: DEFAULT					) ? bs : alt.bs,
				( alt.ms == sampler			:: DEFAULT					) ? ms : alt.ms,
				( alt.ss == stencil			:: DEFAULT					) ? ss : alt.ss,
				( alt.ps == UNDEFINED_CSO	|| alt.ps == NULL_SHADER	) ? ps : alt.ps,
				( alt.vs == UNDEFINED_CSO	|| alt.vs == NULL_SHADER	) ? vs : alt.vs,
				( alt.gs == UNDEFINED_CSO	|| alt.gs == NULL_SHADER	) ? gs : alt.gs,
			};
		}

		inline render_settings& operator&= (const render_settings& alt) { *this = operator&(alt); return *this; }

	private:
		void set_state(const rasterize	::state& s) { rs = s; }
		void set_state(const blend		::state& s) { bs = s; }
		void set_state(const stencil	::state& s) { ss = s; }
		void set_state(const sampler	::state& s) { ms = s; }
		void set_state(const pixel_shader&		 s) { ps = s; }
		void set_state(const vertex_shader&		 s) { vs = s; }
		void set_state(const geometry_shader&	 s) { gs = s; }

	};
}