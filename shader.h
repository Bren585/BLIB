#pragma once
#include <d3d11.h>
#include <wrl.h>
#include "string.h"

#define UNDEFINED_CSO string{"-1"}
#define DEFAULT_FLAT "default_flat"
#define DEFAULT_FULL "default_full"
#define NULL_SHADER	 "null"

namespace BLIB {
	namespace shader {
		namespace _private {
			struct shader_base {
				string cso;
				operator string() const { return cso; }
				bool operator==(const string& s) const { return cso == s; }
				bool operator!=(const string& s) const { return cso != s; }
			protected:
				shader_base(string name) : cso(name) {}
			};
		}
	}

	struct pixel_shader		: shader::_private::shader_base { explicit pixel_shader		(string name) : shader_base(name) {} };
	struct vertex_shader	: shader::_private::shader_base { explicit vertex_shader	(string name) : shader_base(name) {} };
	struct geometry_shader	: shader::_private::shader_base { explicit geometry_shader	(string name) : shader_base(name) {} };

	namespace shader {
		void set_filepath(const string& path);
		
		void load_flat(const string& vs_name, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);

		void load_full(const string& vs_name, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);

		void load_vs(const string& vs_name, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);

		void set_vs(const string& vs_name);
	
		void load_ps(const string& ps_name);

		void set_ps(const string& ps_name);

		void load_gs(const string& gs_name);

		void set_gs(const string& gs_name);

		void release_all();
	}

}