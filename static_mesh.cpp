#include "pch.h"
#include "static_mesh.h"
#include "geometric_primitive.h"
#include "constant_buffer_indices.h"
#include <fstream>

using namespace BLIB;
using std::vector;

static_mesh::static_mesh(const string& obj_filename, bool uv_inverse = false) {
	vector<vertex> vertices;
	vector<uint32_t> indices;
	uint32_t current_index{ 0 };

	vector<float3> positions;
	vector<float3> normals;
	vector<float2> texcoords;
	vector<int>	   tangent_counts;
	vector<string> mtl_filenames;

	maximum = float3{ 0 };
	minimum = float3{ 0 };

	std::wifstream fin(obj_filename.wide());
	_ASSERT_EXPR(fin, L"OBJ file not found.");
	wchar_t command[256];

	/* Mesh Data Loading */ {
		while (fin) {
			fin >> command;
			if (0 == wcscmp(command, L"v")) {
				float x, y, z;
				fin >> x >> y >> z;
				update_maximum(maximum, float3(x, y, z));
				update_minimum(minimum, float3(x, y, z));
				positions.push_back({ x, y, z });
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"vn")) {
				float i, j, k;
				fin >> i >> j >> k;
				normals.push_back({ i, j, k });
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"vt")) {
				float u, v;
				fin >> u >> v;
				texcoords.push_back({ u, (uv_inverse ? 1.0f - v : v) });
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"f")) {
				for (size_t i = 0; i < 3; i++) {
					vertex vertex;
					size_t v, vt, vn;

					fin >> v;
					vertex.position = positions.at(v - 1);
					if (L'/' == fin.peek()) {
						fin.ignore(1);
						if (L'/' != fin.peek()) {
							fin >> vt;
							vertex.texcoord = texcoords.at(vt - 1);
						}
						if (L'/' == fin.peek()) {
							fin.ignore(1);
							fin >> vn;
							vertex.normal = normals.at(vn - 1);
						}
					}
					vertices.push_back(vertex);
					indices.push_back(current_index++);
				}
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"mtllib")) {
				wchar_t mtllib[256];
				fin >> mtllib;
				mtl_filenames.push_back(mtllib);
			}
			else if (0 == wcscmp(command, L"usemtl")) {
				wchar_t usemtl[MAX_PATH]{ 0 };
				fin >> usemtl;
				subset& new_subset{ subsets.emplace_back() };
				new_subset.material_name = usemtl;
				new_subset.start_index_location = static_cast<uint32_t>(indices.size());
			}
			else {
				fin.ignore(1024, L'\n');
			}
		}
		fin.close();

		// Generate Tangents

		tangent_counts.resize(vertices.size(), 0);

		for (int i = 0; i < indices.size(); i += 3) {
			int i0 = i;
			int i1 = i + 1;
			int i2 = i + 2;

			vertex& v0 = vertices[indices[i0]];
			vertex& v1 = vertices[indices[i1]];
			vertex& v2 = vertices[indices[i2]];

			float3 deltaPos1	= v1.position - v0.position;
			float3 deltaPos2	= v2.position - v0.position;
			float2 deltaUV1		= v1.texcoord - v0.texcoord;
			float2 deltaUV2		= v2.texcoord - v0.texcoord;

			float3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) / (deltaUV1 % deltaUV2);
			v0.tangent += tangent;
			v1.tangent += tangent; 
			v2.tangent += tangent; 
			tangent_counts[i0]++;
			tangent_counts[i1]++;
			tangent_counts[i2]++;
		}

		for (int i = 0; i < vertices.size(); i++) {
			float4& tangent = vertices[i].tangent;
			tangent /= tangent_counts[i];
			tangent.norm();
			tangent.w = 1; // Assume by convention
		}

		create_buffers();
	}

	/* Subset Range Cleanup */ {
		std::vector<subset>::reverse_iterator rit = subsets.rbegin();
		rit->index_count = static_cast<uint32_t>(indices.size() - rit->start_index_location);
		for (++rit; rit != subsets.rend(); ++rit) {
			rit->index_count = (rit - 1)->start_index_location - rit->start_index_location;
		}
	}

	uint32_t material_index = -1;
	std::unordered_map<int, phong> phong_mats;

	/* Phong Material Data Loading */ {
		const string filepath = obj_filename.get_filepath();
		const string mtl_filename = filepath + mtl_filenames[0].get_filename();

		fin.open(mtl_filename.wide());
		//_ASSERT_EXPR(fin, "MTL file not found.");

		while (fin) {
			fin >> command;
			if (0 == wcscmp(command, L"map_Kd")) {
				fin.ignore();
				wchar_t map_Kd[256];
				fin >> map_Kd;
				
				phong_mats[material_index].texture_filenames[0] = filepath + string(map_Kd).get_filename();
				fin.ignore(1024, L'\n');
			}
			else if (0 == wcscmp(command, L"map_bump") || 0 == wcscmp(command, L"bump")) {
				fin.ignore();
				wchar_t map_bump[256];
				fin >> map_bump;

				phong_mats[material_index].texture_filenames[1] = filepath + string(map_bump).get_filename();
				fin.ignore(1024, '\n');
			}
			else if (0 == wcscmp(command, L"newmtl")) {
				fin.ignore();
				wchar_t newmtl[256];
				phong material;
				fin >> newmtl;
				material.name = newmtl;
				phong_mats.insert(std::make_pair(++material_index, material));
			}
			else if (0 == wcscmp(command, L"Kd")) {
				float r, g, b;
				fin >> r >> g >> b;
				phong_mats[material_index].Kd = { r, g, b, 1 };
				fin.ignore(1204, L'\n');
			}
			else {
				fin.ignore(1024, L'\n');
			}
		}
		fin.close();

		if (phong_mats.size() == 0) {
			for (const subset& subset : subsets) {
				phong new_material;
				new_material.name = subset.material_name;
				phong_mats.insert(std::make_pair(++material_index, std::move(new_material)));
			}
		}
	}

	/* Material Texture Loading And PBR Conversion*/ {
		for (uint32_t i = 0; i < material_index; i++) {
			phong& phong_mtl = phong_mats[i];
			material pbr_mtl;

			if (phong_mtl.texture_filenames[texture_type::texture_map] != "") {
				pbr_mtl.textures[texture_type::texture_map] = std::make_unique<material_texture_file>(phong_mtl.texture_filenames[texture_type::texture_map]);
			}

			if (phong_mtl.texture_filenames[texture_type::normal_map] != "") {
				pbr_mtl.textures[texture_type::normal_map] = std::make_unique<material_texture_file>(phong_mtl.texture_filenames[texture_type::normal_map]);
			}

			construct_pbr_from_phong(&pbr_mtl, phong_mtl.Ka, phong_mtl.Kd, phong_mtl.Ks);
			pbr_mtl.construct();

			pbr_mtl.unique_id	= phong_mtl.unique_id;
			pbr_mtl.name		= phong_mtl.name;

			materials.emplace(pbr_mtl.unique_id, std::move(pbr_mtl));
		}
	}

	input_element_desc = {
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	create_shaders("default_full");
}

void static_mesh::render(const float4x4& world, const color& material_color) const {
	uint32_t stride{ sizeof(vertex) };
	uint32_t offset{ 0 };

	device::context()->IASetVertexBuffers(0, 1, get_vertices(), &stride, &offset);
	device::context()->IASetIndexBuffer(get_indices(), DXGI_FORMAT_R32_UINT, 0);
	device::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (const auto& material_pair : materials) {
		const material& material{ material_pair.second };

		material.bind(0);

		constants data{ world, material_color };
		device::context()->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
		device::context()->VSSetConstantBuffers(FULL_VS_CB, 1, constant_buffer.GetAddressOf());

		for (const subset& subset : subsets) {
			if (material.name == subset.material_name) {
				device::context()->DrawIndexed(subset.index_count, subset.start_index_location, 0);
			}
		}
	}
}

void static_mesh::render_bounds(const float4x4& world, const color& material_color) {
	if (!bounding_box) bounding_box.reset(create_cube(minimum, maximum)); 
	bounding_box->render(world, material_color);
}

static_mesh::~static_mesh() { }