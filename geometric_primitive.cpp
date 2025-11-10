#include "pch.h"
#include "geometric_primitive.h"
#include "texture.h"

using namespace BLIB;

geometric_primitive::geometric_primitive() {
	input_element_desc = {
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	create_shaders("default_full");
	create_materials();

	minimum = -0.5f;
	maximum = 0.5f;
}

void geometric_primitive::render(const float4x4& world, const color& material_color) const {
	uint32_t stride{ sizeof(vertex) };
	uint32_t offset{ 0 };

	device::context()->IASetVertexBuffers(0, 1, get_vertices(), &stride, &offset);
	device::context()->IASetIndexBuffer(get_indices(), DXGI_FORMAT_R32_UINT, 0);
	device::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	constants data{ world, material_color };
	device::context()->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
	device::context()->VSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

	materials.at(0).bind(0);

	D3D11_BUFFER_DESC buffer_desc;
	get_indices()->GetDesc(&buffer_desc);
	device::context()->DrawIndexed(buffer_desc.ByteWidth / sizeof(uint32_t), 0, 0);
}

void geometric_primitive::create_materials() {
	material mat;
	mat.name = "primitive_material";
	mat.unique_id = 0;
	mat.construct();

	materials.emplace(mat.unique_id, std::move(mat));
}

quad::quad() {
	vertices = {
		{ { -0.5f,  0.5f, 0.0f }, { 0, 0, -1 }, { 0, 0 }, { 1, 0, 0, -1 } },
		{ {  0.5f,  0.5f, 0.0f }, { 0, 0, -1 }, { 1, 0 }, { 1, 0, 0, -1 } },
		{ { -0.5f, -0.5f, 0.0f }, { 0, 0, -1 }, { 0, 1 }, { 1, 0, 0, -1 } },
		{ {  0.5f, -0.5f, 0.0f }, { 0, 0, -1 }, { 1, 1 }, { 1, 0, 0, -1 } }
	};

	indices = { 0, 1, 2, 2, 1, 3 };

	create_buffers();
}

cube::cube() : geometric_primitive() {
	vertices = {
		// front
		{ { -0.5f, -0.5f, -0.5f }, {  0,  0, -1 }, { 0, 0 }, {  1,  0,  0,  1 } }, // 0         3 ---- 2 
		{ {  0.5f, -0.5f, -0.5f }, {  0,  0, -1 }, { 1, 0 }, {  1,  0,  0,  1 } }, // 1         |      |
		{ {  0.5f,  0.5f, -0.5f }, {  0,  0, -1 }, { 1, 1 }, {  1,  0,  0,  1 } }, // 2         |      |
		{ { -0.5f,  0.5f, -0.5f }, {  0,  0, -1 }, { 0, 1 }, {  1,  0,  0,  1 } }, // 3         0 ---- 1
		// right
		{ {  0.5f, -0.5f, -0.5f }, {  1,  0,  0 }, { 0, 0 }, {  0,  0,  1,  1 } }, // 4  (1)    2 ---- 6
		{ {  0.5f,  0.5f, -0.5f }, {  1,  0,  0 }, { 1, 0 }, {  0,  0,  1,  1 } }, // 5  (2)    |      |
		{ {  0.5f, -0.5f,  0.5f }, {  1,  0,  0 }, { 1, 1 }, {  0,  0,  1,  1 } }, // 6  (5)    |      |
		{ {  0.5f,  0.5f,  0.5f }, {  1,  0,  0 }, { 0, 1 }, {  0,  0,  1,  1 } }, // 7  (6)    1 ---- 5
		// back
		{ { -0.5f, -0.5f,  0.5f }, {  0,  0,  1 }, { 0, 0 }, { -1,  0,  0,  1 } }, // 8  (4)    6 ---- 7
		{ {  0.5f, -0.5f,  0.5f }, {  0,  0,  1 }, { 1, 0 }, { -1,  0,  0,  1 } }, // 9  (5)    |      |
		{ {  0.5f,  0.5f,  0.5f }, {  0,  0,  1 }, { 1, 1 }, { -1,  0,  0,  1 } }, // 10 (6)    |      |
		{ { -0.5f,  0.5f,  0.5f }, {  0,  0,  1 }, { 0, 1 }, { -1,  0,  0,  1 } }, // 11 (7)    5 ---- 4
		// left 
		{ { -0.5f, -0.5f, -0.5f }, { -1,  0,  0 }, { 0, 0 }, {  0,  0, -1,  1 } }, // 12 (0)    7 ---- 3
		{ { -0.5f,  0.5f, -0.5f }, { -1,  0,  0 }, { 1, 0 }, {  0,  0, -1,  1 } }, // 13 (3)    |      |
		{ { -0.5f, -0.5f,  0.5f }, { -1,  0,  0 }, { 1, 1 }, {  0,  0, -1,  1 } }, // 14 (4)    |      |
		{ { -0.5f,  0.5f,  0.5f }, { -1,  0,  0 }, { 0, 1 }, {  0,  0, -1,  1 } }, // 15 (7)    4 ---- 0
		// down
		{ { -0.5f, -0.5f, -0.5f }, {  0, -1,  0 }, { 0, 0 }, {  1,  0,  0,  1 } }, // 16 (0)    0 ---- 1
		{ {  0.5f, -0.5f, -0.5f }, {  0, -1,  0 }, { 1, 0 }, {  1,  0,  0,  1 } }, // 17 (1)    |      |
		{ { -0.5f, -0.5f,  0.5f }, {  0, -1,  0 }, { 1, 1 }, {  1,  0,  0,  1 } }, // 18 (4)    |      |
		{ {  0.5f, -0.5f,  0.5f }, {  0, -1,  0 }, { 0, 1 }, {  1,  0,  0,  1 } }, // 19 (5)    4 ---- 5
		// up
		{ {  0.5f,  0.5f, -0.5f }, {  0,  1,  0 }, { 0, 0 }, { -1,  0,  0,  1 } }, // 20 (2)    7 ---- 6
		{ { -0.5f,  0.5f, -0.5f }, {  0,  1,  0 }, { 1, 0 }, { -1,  0,  0,  1 } }, // 21 (3)    |      |
		{ {  0.5f,  0.5f,  0.5f }, {  0,  1,  0 }, { 1, 1 }, { -1,  0,  0,  1 } }, // 22 (6)    |      |
		{ { -0.5f,  0.5f,  0.5f }, {  0,  1,  0 }, { 0, 1 }, { -1,  0,  0,  1 } }, // 23 (7)    3 ---- 2
	};

	indices = {
		 2,  1,  0,  0,  3,  2,
		 7,  6,  4,  4,  5,  7,
		11,  8,  9,  9, 10, 11,
		13, 12, 14, 14, 15, 13,
		17, 19, 18, 18, 16, 17,
		22, 20, 21, 21, 23, 22
	};

	create_buffers();
}

custom_cube::custom_cube(float3 min, float3 max) : geometric_primitive() {
	minimum = min;
	maximum = max;
	
	vertices = {
		// front + 0
		{ { min.x, min.y, min.z }, {  0,  0, -1 }, { 0, 0 }, {  1,  0,  0,  1 } }, // 0         3 ---- 2 
		{ { max.x, min.y, min.z }, {  0,  0, -1 }, { 1, 0 }, {  1,  0,  0,  1 } }, // 1         |      |
		{ { max.x, max.y, min.z }, {  0,  0, -1 }, { 1, 1 }, {  1,  0,  0,  1 } }, // 2         |      |
		{ { min.x, max.y, min.z }, {  0,  0, -1 }, { 0, 1 }, {  1,  0,  0,  1 } }, // 3         0 ---- 1
		// right + 4
		{ { max.x, min.y, min.z }, {  1,  0,  0 }, { 0, 0 }, {  0,  0,  1,  1 } }, // 4  (1)    2 ---- 6
		{ { max.x, max.y, min.z }, {  1,  0,  0 }, { 1, 0 }, {  0,  0,  1,  1 } }, // 5  (2)    |      |
		{ { max.x, min.y, max.z }, {  1,  0,  0 }, { 1, 1 }, {  0,  0,  1,  1 } }, // 6  (5)    |      |
		{ { max.x, max.y, max.z }, {  1,  0,  0 }, { 0, 1 }, {  0,  0,  1,  1 } }, // 7  (6)    1 ---- 5
		// back
		{ { min.x, min.y, max.z }, {  0,  0,  1 }, { 0, 0 }, { -1,  0,  0,  1 } }, // 8  (4)    6 ---- 7
		{ { max.x, min.y, max.z }, {  0,  0,  1 }, { 1, 0 }, { -1,  0,  0,  1 } }, // 9  (5)    |      |
		{ { max.x, max.y, max.z }, {  0,  0,  1 }, { 1, 1 }, { -1,  0,  0,  1 } }, // 10 (6)    |      |
		{ { min.x, max.y, max.z }, {  0,  0,  1 }, { 0, 1 }, { -1,  0,  0,  1 } }, // 11 (7)    5 ---- 4
		// left
		{ { min.x, min.y, min.z }, { -1,  0,  0 }, { 0, 0 }, {  0,  0, -1,  1 } }, // 12 (0)    7 ---- 3
		{ { min.x, max.y, min.z }, { -1,  0,  0 }, { 1, 0 }, {  0,  0, -1,  1 } }, // 13 (3)    |      |
		{ { min.x, min.y, max.z }, { -1,  0,  0 }, { 1, 1 }, {  0,  0, -1,  1 } }, // 14 (4)    |      |
		{ { min.x, max.y, max.z }, { -1,  0,  0 }, { 0, 1 }, {  0,  0, -1,  1 } }, // 15 (7)    4 ---- 0
		// down
		{ { min.x, min.y, min.z }, {  0, -1,  0 }, { 0, 0 }, {  1,  0,  0,  1 } }, // 16 (0)    0 ---- 1
		{ { max.x, min.y, min.z }, {  0, -1,  0 }, { 1, 0 }, {  1,  0,  0,  1 } }, // 17 (1)    |      |
		{ { min.x, min.y, max.z }, {  0, -1,  0 }, { 1, 1 }, {  1,  0,  0,  1 } }, // 18 (4)    |      |
		{ { max.x, min.y, max.z }, {  0, -1,  0 }, { 0, 1 }, {  1,  0,  0,  1 } }, // 19 (5)    4 ---- 5
		// up
		{ { max.x, max.y, min.z }, {  0,  1,  0 }, { 0, 0 }, { -1,  0,  0,  1 } }, // 20 (2)    7 ---- 6
		{ { min.x, max.y, min.z }, {  0,  1,  0 }, { 1, 0 }, { -1,  0,  0,  1 } }, // 21 (3)    |      |
		{ { max.x, max.y, max.z }, {  0,  1,  0 }, { 1, 1 }, { -1,  0,  0,  1 } }, // 22 (6)    |      |
		{ { min.x, max.y, max.z }, {  0,  1,  0 }, { 0, 1 }, { -1,  0,  0,  1 } }, // 23 (7)    3 ---- 2
	}; 

	indices = {
		 2,  1,  0,  0,  3,  2,
		 7,  6,  4,  4,  5,  7,
		11,  8,  9,  9, 10, 11,
		13, 12, 14, 14, 15, 13,
		17, 19, 18, 18, 16, 17,
		22, 20, 21, 21, 23, 22
	};

	create_buffers();
}

cylinder::cylinder(int edges) : geometric_primitive() {
	// Given a circle with x edges...
	// each edge creates 1 triangle on top and 1 on bottom, then two for the connecting face
	// so, 4 triangles per edge.
	// each edge requires access to a center set of vertices (top center and bottom center)
	// each edge also requires four additional vertices for each face
	// and additional two vertices for both top and bottom triangles
	// so total eight vertices

	assert(edges > 2);

	int vertex_count = edges * 8 + 2; // eight vertecies per edge + center vertices
	int index_count = edges * 4 * 3; // four traingles per edge * three indices per triangle

	vertices.resize(vertex_count);
	indices.resize(index_count);

	vertices[0] = { { 0.0f, -0.5f, 0.0f }, { 0, -1, 0 }, { 0, 0 }, { 1, 0, 0, 1 }}; // bottom
	vertices[1] = { { 0.0f,  0.5f, 0.0f }, { 0,  1, 0 }, { 0, 0 }, { 1, 0, 0, 1 }}; // top

	int vi = 2; // vertices index
	int ii = 0; // indices index

	for (int edge = 0; edge < edges; edge++) {
		float a1 = (float) edge      / (float)edges * DirectX::XM_2PI;
		float a2 = (float)(edge + 1) / (float)edges * DirectX::XM_2PI;

		int sv = vi; // start vertex

		float ca1 = cosf(a1);
		float ca2 = cosf(a2);
		float sa1 = sinf(a1);
		float sa2 = sinf(a2);

		// Faces
		vertices[vi++] = { { ca1 / 2.0f, -0.5f, sa1 / 2.0f }, { ca1, 0, sa1 }, { 0, 0 }, { -sa1, 0, ca1, 1 } }; // 0   1 ---- 3
		vertices[vi++] = { { ca1 / 2.0f,  0.5f, sa1 / 2.0f }, { ca1, 0, sa1 }, { 0, 0 }, { -sa1, 0, ca1, 1 } }; // 1   |   ,' |
		vertices[vi++] = { { ca2 / 2.0f, -0.5f, sa2 / 2.0f }, { ca2, 0, sa2 }, { 0, 0 }, { -sa2, 0, ca2, 1 } }; // 2   | ,'   |
		vertices[vi++] = { { ca2 / 2.0f,  0.5f, sa2 / 2.0f }, { ca2, 0, sa2 }, { 0, 0 }, { -sa2, 0, ca2, 1 } }; // 3   0 ---- 2

		// Top and Bottom
		vertices[vi++] = { { ca1 / 2.0f, -0.5f, sa1 / 2.0f }, { 0, -1, 0 }, { 0, 0 }, { 1, 0, 0, 1 } }; // 4
		vertices[vi++] = { { ca2 / 2.0f, -0.5f, sa2 / 2.0f }, { 0, -1, 0 }, { 0, 0 }, { 1, 0, 0, 1 } }; // 5    4 > 5 > bottom center
		vertices[vi++] = { { ca1 / 2.0f,  0.5f, sa1 / 2.0f }, { 0,  1, 0 }, { 0, 0 }, { 1, 0, 0, 1 } }; // 6    7 > 6 > top center
		vertices[vi++] = { { ca2 / 2.0f,  0.5f, sa2 / 2.0f }, { 0,  1, 0 }, { 0, 0 }, { 1, 0, 0, 1 } }; // 7 

		// Faces
		indices[ii++] = sv + 3;
		indices[ii++] = sv + 2;
		indices[ii++] = sv + 0;
		indices[ii++] = sv + 0;
		indices[ii++] = sv + 1;
		indices[ii++] = sv + 3;

		// Top and Bottom
		indices[ii++] = sv + 4;
		indices[ii++] = sv + 5;
		indices[ii++] = 0;
		indices[ii++] = sv + 7;
		indices[ii++] = sv + 6;
		indices[ii++] = 1;
	}

	assert(vertex_count == vi);
	assert(index_count == ii);

	create_buffers();
}

#pragma warning( push )
#pragma warning( disable : 6386 )
sphere::sphere(int edges) : geometric_primitive() {
	// Using the same principles as a cylinder...
	// We can create strip faces in a circle around the center
	// They converge at the top and bottom
	// kinda like this...

	//       T
	//     ,' ',
	//    0 --- X + 1
	//    |  ,*'|
	//    |,*   |
	//    1 --- X + 2
	//    |     |
	//      ...
	//    |     |
	//    X --- 2X
	//     ', ,'
	//       B

	// This means we have the two triangles at top and bottom like before (4 vertices, 2 triangles)
	// Then the first and last "edges" are by necessity used to make the top and bottom
	// and for every edge past that we need two more triangles (4 vertices, 2 triangles)

	assert(edges > 2);

	int vertex_count = 2 + (edges) * (4 + 4 * (edges - 3)); // four vertecies for top and bottom, then four per edge per edge, plus two in the center
	int index_count = (2 * edges * (edges - 2)) * 3; // two triangles for top and bottom + two triangles per edge per edge * three indices per triangle

	vertices.resize(vertex_count);
	indices.resize(index_count);

	vertices[0] = { { 0.0f, -0.5f, 0.0f }, { 0, -1, 0 }, { 0, 0 }, { 1, 0, 0,  1 } }; // bottom
	vertices[1] = { { 0.0f,  0.5f, 0.0f }, { 0,  1, 0 }, { 0, 0 }, { 1, 0, 0, -1 } }; // top

	int vi = 2; // vertices index
	int ii = 0; // indices index

	for (int hedge = 0; hedge < edges; hedge++) {
		float ax1 = (float) hedge		/ (float)edges * DirectX::XM_2PI;
		float ax2 = (float)(hedge + 1)	/ (float)edges * DirectX::XM_2PI;

		float cax1 = cosf(ax1);
		float cax2 = cosf(ax2);
		float sax1 = sinf(ax1);
		float sax2 = sinf(ax2);

		for (int vedge = 0; vedge < edges - 1; vedge++) {
			if (vedge == 0 /* Bottom */) {
				int sv = vi; // start vertex

				float ay2 = (float)(vedge + 1) / ((float)edges - 1) * DirectX::XM_PI;
				float say2 = sinf(ay2);

				float x1y2 =  say2 * cax1;
				float x2y2 =  say2 * cax2;
				float y2   = -cosf(ay2);
				float z1y2 =  say2 * sax1;
				float z2y2 =  say2 * sax2;

				vertices[vi++] = { { x1y2 / 2.0f, y2 / 2.0f, z1y2 / 2.0f }, { x1y2, y2, z1y2 }, { 0, 0 }, { -sax1, 0, cax1, 1 } }; // 0
				vertices[vi++] = { { x2y2 / 2.0f, y2 / 2.0f, z2y2 / 2.0f }, { x2y2, y2, z2y2 }, { 0, 0 }, { -sax2, 0, cax2, 1 } }; // 1    0 > 1 > bottom 

				indices[ii++] = sv + 0;
				indices[ii++] = sv + 1;
				indices[ii++] = 0;
			}
			else if (vedge == edges - 2 /* Top */) {
				int sv = vi; // start vertex

				float ay1 = (float) vedge / ((float)edges - 1) * DirectX::XM_PI;
				float say1 = sinf(ay1);

				float x1y1 =  say1 * cax1;
				float x2y1 =  say1 * cax2;
				float y1   = -cosf(ay1);
				float z1y1 =  say1 * sax1;
				float z2y1 =  say1 * sax2;

				vertices[vi++] = { { x1y1 / 2.0f, y1 / 2.0f, z1y1 / 2.0f }, { x1y1, y1, z1y1 }, { 0, 0 }, { -sax1, 0, cax1, 1 } }; // 0
				vertices[vi++] = { { x2y1 / 2.0f, y1 / 2.0f, z2y1 / 2.0f }, { x2y1, y1, z2y1 }, { 0, 0 }, { -sax2, 0, cax2, 1 } }; // 1    1 > 0 > top 

				indices[ii++] = sv + 1;
				indices[ii++] = sv + 0;
				indices[ii++] = 1;
			}
			else {
				int sv = vi; // start vertex

				float ay1 = (float) vedge      / ((float)edges - 1) * DirectX::XM_PI;
				float ay2 = (float)(vedge + 1) / ((float)edges - 1) * DirectX::XM_PI;
				float say1 = sinf(ay1);
				float say2 = sinf(ay2);

				float x1y1 =  say1 * cax1;
				float x2y1 =  say1 * cax2;
				float x1y2 =  say2 * cax1;
				float x2y2 =  say2 * cax2;
				float y1   = -cosf(ay1);
				float y2   = -cosf(ay2);
				float z1y1 =  say1 * sax1;
				float z2y1 =  say1 * sax2;
				float z1y2 =  say2 * sax1;
				float z2y2 =  say2 * sax2;

				//float w1 = (y1 < 0) ? 1.0f : 1.0f;
				//float w2 = (y2 < 0) ? 1.0f : 1.0f;

				vertices[vi++] = { { x1y1 / 2.0f, y1 / 2.0f, z1y1 / 2.0f }, { x1y1, y1, z1y1 }, { 0, 0 }, { -sax1, 0, cax1, 1 } }; // 0   1 ---- 3
				vertices[vi++] = { { x1y2 / 2.0f, y2 / 2.0f, z1y2 / 2.0f }, { x1y2, y2, z1y2 }, { 0, 0 }, { -sax1, 0, cax1, 1 } }; // 1   |   ,' |
				vertices[vi++] = { { x2y1 / 2.0f, y1 / 2.0f, z2y1 / 2.0f }, { x2y1, y1, z2y1 }, { 0, 0 }, { -sax2, 0, cax2, 1 } }; // 2   | ,'   |
				vertices[vi++] = { { x2y2 / 2.0f, y2 / 2.0f, z2y2 / 2.0f }, { x2y2, y2, z2y2 }, { 0, 0 }, { -sax2, 0, cax2, 1 } }; // 3   0 ---- 2

				indices[ii++] = sv + 3;
				indices[ii++] = sv + 2;
				indices[ii++] = sv + 0;
				indices[ii++] = sv + 0;
				indices[ii++] = sv + 1;
				indices[ii++] = sv + 3;
			}
		}
	}

	assert(vertex_count == vi);
	assert(index_count == ii);

	create_buffers();
}

capsule::capsule(float height, float radius, int edges) : geometric_primitive() {
	assert(edges > 2);

	int vertex_count = 2 + (4 * edges) + (edges * (4 + 4 * (edges - 2 - edges % 2))); // center, cylinder faces only, sphere with an aditional vedge or two for splitting in half.
	int index_count = (2 * edges * (edges - edges % 2)) * 3; // Same as sphere but with two more triangles per edge for the cylinder

	vertices.resize(vertex_count);
	indices.resize(index_count);

	vertices[0] = { { 0.0f, -height - radius, 0.0f }, { 0, -1, 0 }, { 0, 0 }, { 1, 0, 0, 1 } }; // bottom
	vertices[1] = { { 0.0f,  height + radius, 0.0f }, { 0,  1, 0 }, { 0, 0 }, { 1, 0, 0, 1 } }; // top

	int vi = 2; // vertices index
	int ii = 0; // indices index

	for (int hedge = 0; hedge < edges; hedge++) {
		float ax1 = (float)hedge / (float)edges * DirectX::XM_2PI;
		float ax2 = (float)(hedge + 1) / (float)edges * DirectX::XM_2PI;

		float cax1 = cosf(ax1);
		float cax2 = cosf(ax2);
		float sax1 = sinf(ax1);
		float sax2 = sinf(ax2);

		/* cylinder */ {
			int sv = vi; // start vertex

			vertices[vi++] = { { cax1 * radius, -height, sax1 * radius }, { cax1, 0, sax1 }, { 0, 0 }, { -sax1, 0, cax1, 1 } }; // 0   1 ---- 3
			vertices[vi++] = { { cax1 * radius,  height, sax1 * radius }, { cax1, 0, sax1 }, { 0, 0 }, { -sax1, 0, cax1, 1 } }; // 1   |   ,' |
			vertices[vi++] = { { cax2 * radius, -height, sax2 * radius }, { cax2, 0, sax2 }, { 0, 0 }, { -sax2, 0, cax2, 1 } }; // 2   | ,'   |
			vertices[vi++] = { { cax2 * radius,  height, sax2 * radius }, { cax2, 0, sax2 }, { 0, 0 }, { -sax2, 0, cax2, 1 } }; // 3   0 ---- 2

			indices[ii++] = sv + 3;
			indices[ii++] = sv + 2;
			indices[ii++] = sv + 0;
			indices[ii++] = sv + 0;
			indices[ii++] = sv + 1;
			indices[ii++] = sv + 3;
		}

		for (int up = -1; up < 2; up += 2) {
			float w = -static_cast<float>(up);
			for (int vedge = 0; vedge < edges / 2; vedge++) {
				if (vedge == 0) {
					/* Bottom */
					int sv = vi; // start vertex

					float ay2 = (float)(vedge + 1) / ((float)edges - 1) * DirectX::XM_PI;
					float say2 = sinf(ay2);

					float x1y2 = say2 * cax1;
					float x2y2 = say2 * cax2;
					float y2   = up * cosf(ay2);
					float z1y2 = say2 * sax1;
					float z2y2 = say2 * sax2;

					vertices[vi++] = { { x1y2 * radius, up * height + y2 * radius, z1y2 * radius }, { x1y2, y2, z1y2 }, { 0, 0 }, { -sax1, 0, cax1, w } }; // 0
					vertices[vi++] = { { x2y2 * radius, up * height + y2 * radius, z2y2 * radius }, { x2y2, y2, z2y2 }, { 0, 0 }, { -sax1, 0, cax2, w } }; // 1    0 > 1 > bottom 

					if (up == -1) {
						indices[ii++] = sv + 0;
						indices[ii++] = sv + 1;
						indices[ii++] = 0;
					}
					else {
						indices[ii++] = sv + 1;
						indices[ii++] = sv + 0;
						indices[ii++] = 1;
					}
					
				}
				else {
					int sv = vi; // start vertex

					float ay1 = (float) vedge		/ ((float)edges - 1) * DirectX::XM_PI;
					float ay2 = (float)(vedge + 1)	/ ((float)edges - 1) * DirectX::XM_PI;

					if (vedge == edges / 2 - 1 && !(edges % 2)) {
						ay2 = DirectX::XM_PIDIV2;
					}

					float say1 = sinf(ay1);
					float say2 = sinf(ay2);

					float x1y1 = say1 * cax1;
					float x2y1 = say1 * cax2;
					float x1y2 = say2 * cax1;
					float x2y2 = say2 * cax2;
					float y1   = up * cosf(ay1);
					float y2   = up * cosf(ay2);
					float z1y1 = say1 * sax1;
					float z2y1 = say1 * sax2;
					float z1y2 = say2 * sax1;
					float z2y2 = say2 * sax2;

					vertices[vi++] = { { x1y1 * radius, up * height + y1 * radius, z1y1 * radius }, { x1y1, y1, z1y1 }, { 0, 0 }, { -sax1, 0, cax1, w } }; // 0   1 ---- 3
					vertices[vi++] = { { x1y2 * radius, up * height + y2 * radius, z1y2 * radius }, { x1y2, y2, z1y2 }, { 0, 0 }, { -sax1, 0, cax1, w } }; // 1   |   ,' |
					vertices[vi++] = { { x2y1 * radius, up * height + y1 * radius, z2y1 * radius }, { x2y1, y1, z2y1 }, { 0, 0 }, { -sax2, 0, cax2, w } }; // 2   | ,'   |
					vertices[vi++] = { { x2y2 * radius, up * height + y2 * radius, z2y2 * radius }, { x2y2, y2, z2y2 }, { 0, 0 }, { -sax2, 0, cax2, w } }; // 3   0 ---- 2

					if (up == -1) {
						indices[ii++] = sv + 3;
						indices[ii++] = sv + 2;
						indices[ii++] = sv + 0;
						indices[ii++] = sv + 0;
						indices[ii++] = sv + 1;
						indices[ii++] = sv + 3;
					}
					else {
						indices[ii++] = sv + 3;
						indices[ii++] = sv + 1;
						indices[ii++] = sv + 0;
						indices[ii++] = sv + 0;
						indices[ii++] = sv + 2;
						indices[ii++] = sv + 3;
					}
					
				}
			}
		}
	}

	assert(vertex_count == vi);
	assert(index_count == ii);

	create_buffers();
}
#pragma warning( pop )

rect_pyramid::rect_pyramid() {
	float rt_five = sqrtf(5.0f);
	float dy = 1 / rt_five;
	float dx = dy * 2;

	vertices = {
		// Bottom
		{ { -0.5f, -0.5f, -0.5f }, {  00, -01,  00 }, { 0, 0 }, {  1,  0,  0,  1 } }, // 0 (0)	0 ---- 1
		{ {  0.5f, -0.5f, -0.5f }, {  00, -01,  00 }, { 0, 0 }, {  1,  0,  0,  1 } }, // 1 (1)	|      |
		{ { -0.5f, -0.5f,  0.5f }, {  00, -01,  00 }, { 0, 0 }, {  1,  0,  0,  1 } }, // 2 (2)	|      |
		{ {  0.5f, -0.5f,  0.5f }, {  00, -01,  00 }, { 0, 0 }, {  1,  0,  0,  1 } }, // 3 (3)	2 ---- 3
		// Front
		{ {  0.0f,  0.5f,  0.0f }, {  00,  dy, -dx }, { 0, 0 }, {  1,  0,  0,  1 } }, // 4 (A)	  A
		{ { -0.5f, -0.5f, -0.5f }, {  00,  dy, -dx }, { 0, 0 }, {  1,  0,  0,  1 } }, // 5 (0)	 * *
		{ {  0.5f, -0.5f, -0.5f }, {  00,  dy, -dx }, { 0, 0 }, {  1,  0,  0,  1 } }, // 6 (1)	0 - 1
		// Right
		{ {  0.0f,  0.5f,  0.0f }, {  dx,  dy,  00 }, { 0, 0 }, {  0,  0, -1,  1 } }, // 7 (A)	  A
		{ {  0.5f, -0.5f, -0.5f }, {  dx,  dy,  00 }, { 0, 0 }, {  0,  0, -1,  1 } }, // 8 (1)	 * *
		{ {  0.5f, -0.5f,  0.5f }, {  dx,  dy,  00 }, { 0, 0 }, {  0,  0, -1,  1 } }, // 9 (3)	1 - 3
		// Back
		{ {  0.0f,  0.5f,  0.0f }, {  00,  dy,  dx }, { 0, 0 }, { -1,  0,  0,  1 } }, // 10 (A)	  A
		{ {  0.5f, -0.5f,  0.5f }, {  00,  dy,  dx }, { 0, 0 }, { -1,  0,  0,  1 } }, // 11 (3)	 * *
		{ { -0.5f, -0.5f,  0.5f }, {  00,  dy,  dx }, { 0, 0 }, { -1,  0,  0,  1 } }, // 12 (2)	3 - 2
		// Left
		{ {  0.0f,  0.5f,  0.0f }, { -dx,  dy,  00 }, { 0, 0 }, {  0,  0,  1,  1 } }, // 13 (A)	  A
		{ { -0.5f, -0.5f,  0.5f }, { -dx,  dy,  00 }, { 0, 0 }, {  0,  0,  1,  1 } }, // 14 (2)	 * *
		{ { -0.5f, -0.5f, -0.5f }, { -dx,  dy,  00 }, { 0, 0 }, {  0,  0,  1,  1 } }, // 15 (0)	2 - 0
	};

	indices = {
		1, 3, 2, 
		2, 0, 1,
		4, 6, 5,
		7, 9, 8,
		10, 12, 11,
		13, 15, 14
	};

	create_buffers();
}