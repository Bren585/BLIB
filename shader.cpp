#include "pch.h"
#include "shader.h"

using namespace BLIB;

struct vertex_shader_data {
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
};

std::map<string, vertex_shader_data> vertex_shaders							= { { NULL_SHADER, {nullptr, nullptr}} };
std::map<string, Microsoft::WRL::ComPtr<ID3D11PixelShader>>	pixel_shaders	= { { NULL_SHADER, nullptr } };

string filepath = "-1";

void shader::set_filepath(const string& path) { filepath = path; }

static HRESULT create_vs_from_cso(const string& vs_name, ID3D11VertexShader** vertex_shader, ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements) {
	FILE* fp{ nullptr };
	_ASSERT_EXPR_A(filepath != "-1", "Must Set Filepath");
	fopen_s(&fp, (filepath + vs_name + "_vs.cso"), "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };

	hr = device::get()->CreateVertexShader(cso_data.get(), cso_sz, nullptr, vertex_shader);	VERIFY;

	if (input_layout) {
		hr = device::get()->CreateInputLayout(input_element_desc, num_elements, cso_data.get(), cso_sz, input_layout); VERIFY;
	}

	return hr;
}

static HRESULT create_ps_from_cso(const string& ps_name, ID3D11PixelShader** pixel_shader) {
	FILE* fp{ nullptr };
	_ASSERT_EXPR_A(filepath != "-1", "Must Set Filepath");
	fopen_s(&fp, (filepath + ps_name + "_ps.cso"), "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };

	hr = device::get()->CreatePixelShader(cso_data.get(), cso_sz, nullptr, pixel_shader); VERIFY;

	return hr;
}

void shader::load_vs(const string& vs_name, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements) {
	if (vertex_shaders.try_emplace(vs_name).second) {
		vertex_shader_data& vs_data = vertex_shaders[vs_name];
		create_vs_from_cso(vs_name, vs_data.vertex_shader.GetAddressOf(), vs_data.input_layout.GetAddressOf(), input_element_desc, num_elements);
	}
}

void shader::load_flat(const string& vs_name, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements) {
	load_vs(vs_name, input_element_desc, num_elements);
	load_ps("default_flat");
}

void shader::load_full(const string& vs_name, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements) {
	load_vs(vs_name, input_element_desc, num_elements);
	load_ps("default_full");
}

void shader::set_vs(const string& vs_name) {
	_ASSERT_EXPR_A(vertex_shaders.find(vs_name) != vertex_shaders.end(), L"Vertex Shader Not Loaded");
	device::context()->IASetInputLayout(vertex_shaders[vs_name].input_layout.Get());
	device::context()->VSSetShader(vertex_shaders[vs_name].vertex_shader.Get(), nullptr, 0);
}

void shader::load_ps(const string& ps_name) {
	if (pixel_shaders.try_emplace(ps_name).second) {
		create_ps_from_cso(ps_name, pixel_shaders[ps_name].GetAddressOf());
	}
}

void shader::set_ps(const string& ps_name) {
	load_ps(ps_name);
	device::context()->PSSetShader(pixel_shaders[ps_name].Get(), nullptr, 0);
}