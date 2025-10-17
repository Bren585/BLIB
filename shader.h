#pragma once
#include <d3d11.h>
#include <wrl.h>
#include "string.h"

namespace BLIB {

	namespace shader {
		void set_filepath(const string& path);
		
		void load_flat(const string& vs_name, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);

		void load_full(const string& vs_name, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);

		void load_vs(const string& vs_name, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);

		void set_vs(const string& vs_name);
	
		void load_ps(const string& ps_name);

		void set_ps(const string& ps_name);
	}

}