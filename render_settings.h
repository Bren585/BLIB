#pragma once
#include "blend.h"
#include "rasterize.h"
#include "depth_stencil.h"
#include "sampler.h"
#include "shader.h"

namespace BLIB {
	struct render_settings {
		rasterize	::state	rs = rasterize	::DEFAULT; // rasterize state
		blend		::state	bs = blend		::DEFAULT; // blend state
		stencil		::state	ss = stencil	::DEFAULT; // depth stencil state
		sampler		::state ms = sampler	::DEFAULT; // sampler state
		string				ps = "-1";				   // pixel shader name

		render_settings() = default;
		render_settings(const render_settings&) = default;
		render_settings(render_settings&&) = default;

		template <typename... Args, typename = std::enable_if_t<!(sizeof...(Args) == 1 && std::conjunction_v<std::is_same<render_settings, std::decay_t<Args>>...>)>>
		render_settings(Args&&... args) { 
			(set_state(std::forward<Args>(args)), ...);;
		}

		inline void set() const {
			rasterize	::set(rs);
			blend		::set(bs);
			stencil		::set(ss);
			sampler		::set(ms);

			_ASSERT_EXPR_A(ps != "-1", "Always set a Pixel Shader");
			shader		::set_ps(ps);
		}

		inline render_settings& operator=(const render_settings& o) {
			rs = o.rs;
			bs = o.bs;
			ss = o.ss;
			ms = o.ms;
			ps = o.ps;
			return *this;
		}

		inline render_settings operator&(const render_settings& alt) const {
			return {
				alt.rs == rasterize	::DEFAULT	? rs : alt.rs,
				alt.bs == blend		::DEFAULT	? bs : alt.bs,
				alt.ss == stencil	::DEFAULT	? ss : alt.ss,
				alt.ms == sampler	::DEFAULT	? ms : alt.ms,
				alt.ps == "-1"					? ps : alt.ps,
			};
		}

		inline render_settings& operator&= (const render_settings& alt) { *this = operator&(alt); return *this; }

	private:
		void set_state(const rasterize	::state& s) { rs = s; }
		void set_state(const blend		::state& s) { bs = s; }
		void set_state(const stencil	::state& s) { ss = s; }
		void set_state(const sampler	::state& s) { ms = s; }
		void set_state(const string&			 s) { ps = s; }
		void set_state(const char*				 s) { ps = s; }

	};
}