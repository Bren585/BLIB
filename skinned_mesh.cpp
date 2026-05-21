#include "pch.h"
#include "skinned_mesh.h"
#include "shader.h"
#include "texture.h"
#include "constant_buffer_indices.h"

using namespace BLIB;
using std::vector;

// Helpers

inline DirectX::XMFLOAT4X4 to_xmfloat4x4(const FbxAMatrix& fbxamatrix) {
	DirectX::XMFLOAT4X4 xmf;
	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			xmf.m[r][c] = static_cast<float>(fbxamatrix[r][c]);
		}
	}
	return xmf;
}

inline DirectX::XMFLOAT3 to_xmfloat3(const FbxDouble3& fbxdouble3) {
	DirectX::XMFLOAT3 xmf;
	xmf.x = static_cast<float>(fbxdouble3[0]);
	xmf.y = static_cast<float>(fbxdouble3[1]);
	xmf.z = static_cast<float>(fbxdouble3[2]);
	return xmf;
}

inline DirectX::XMFLOAT4 to_xmfloat4(const FbxDouble4& fbxdouble4) {
	DirectX::XMFLOAT4 xmf;
	xmf.x = static_cast<float>(fbxdouble4[0]);
	xmf.y = static_cast<float>(fbxdouble4[1]);
	xmf.z = static_cast<float>(fbxdouble4[2]);
	xmf.w = static_cast<float>(fbxdouble4[3]);
	return xmf;
}

// Constructor

skinned_mesh::skinned_mesh(const char* fbx_filename, bool triangulate, coordinate_system sys) {
	fbx_name = string(fbx_filename).get_filename().replace_ext("");
	fbx_name[(int)fbx_name.length() - 1] = '_';
	coord_sys = sys;

	FbxManager* fbx_manager{ FbxManager::Create() };
	FbxScene* fbx_scene{ FbxScene::Create(fbx_manager, "") };

	FbxImporter* fbx_importer{ FbxImporter::Create(fbx_manager, "") };
	bool import_status{ false };

	import_status = fbx_importer->Initialize(fbx_filename);
	_ASSERT_EXPR_A(import_status, fbx_importer->GetStatus().GetErrorString());

	import_status = fbx_importer->Import(fbx_scene);
	_ASSERT_EXPR_A(import_status, fbx_importer->GetStatus().GetErrorString());

	FbxGeometryConverter fbx_converter(fbx_manager);
	if (triangulate) {
		fbx_converter.Triangulate(fbx_scene, true/*replace*/, false/*legacy*/);
		fbx_converter.RemoveBadPolygonsFromMeshes(fbx_scene);
	}

	std::function<void(FbxNode*)> traverse{ [&](FbxNode* fbx_node) {
		scene_view::node& node {scene_view.nodes.emplace_back() };
		node.attribute = fbx_node->GetNodeAttribute() ? fbx_node->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
		node.name = fbx_node->GetName();
		node.unique_id = fbx_node->GetUniqueID();
		node.parent_index = scene_view.index_of(fbx_node->GetParent() ? fbx_node->GetParent()->GetUniqueID() : 0);
		for (int child_index = 0; child_index < fbx_node->GetChildCount(); ++child_index) {
			traverse(fbx_node->GetChild(child_index));
		}
	} };

	traverse(fbx_scene->GetRootNode());

	fetch_meshes(fbx_scene);
	fetch_animations(fbx_scene, animations, 0);
	fetch_materials(fbx_filename, fbx_scene);

	fbx_manager->Destroy();

	create_com_objects();

	//scene_view.create_hierarchy();
	//animator.set_hierarchy(scene_view.hierarchy);
}

// Fetching FBX stuff

float4x4 build_relative_transform(FbxNode* node, const FbxNode* root) {
	FbxAMatrix transform = node->EvaluateLocalTransform();

	for (FbxNode* parent = node->GetParent(); parent != root; parent = parent->GetParent()) {
		transform = parent->EvaluateLocalTransform() * transform;
	}

	FbxAMatrix geometric; 
	geometric.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
	geometric.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
	geometric.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));

	return to_xmfloat4x4(transform * geometric);
}

void skinned_mesh::fetch_meshes(FbxScene* fbx_scene) {
	FbxNode* root_node = fbx_scene->GetRootNode();

	for (const scene_view::node& node : scene_view.nodes) {
		if (node.attribute != FbxNodeAttribute::EType::eMesh) continue;

		FbxNode* fbx_node{ fbx_scene->FindNodeByName((const char*)node.name) };
		FbxMesh* fbx_mesh{ fbx_node->GetMesh() };

		mesh& mesh{ meshes.emplace_back() };
		mesh.unique_id = fbx_node->GetUniqueID();
		mesh.name = fbx_node->GetName();
		mesh.node_index = scene_view.index_of(mesh.unique_id);

		mesh.default_global_transform = to_xmfloat4x4(fbx_node->EvaluateGlobalTransform());//build_relative_transform(fbx_node, root_node);
		//DirectX::XMStoreFloat4x4(&mesh.inverse_default_global_transform, DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&mesh.default_global_transform)));

		vector<vector<bone_inf>> bones;
		fetch_bones(fbx_mesh, bones);

		fetch_skeleton(fbx_mesh, mesh.bind_pose);

		if (mesh.bind_pose.bones.empty()) {
			fetch_attachment(fbx_node, mesh.bind_pose);
			if (mesh.bind_pose.bones.empty()) {
				fallback_skeleton(mesh.bind_pose);
			}
		}

		vector<mesh::subset>& subsets{ mesh.subsets };
		const int material_count{ fbx_mesh->GetNode()->GetMaterialCount() };
		subsets.resize(material_count > 0 ? material_count : 1);
		for (int material_index = 0; material_index < material_count; ++material_index) {
			const FbxSurfaceMaterial* fbx_material{ fbx_mesh->GetNode()->GetMaterial(material_index) };
			subsets.at(material_index).material_name = fbx_material->GetName();
			subsets.at(material_index).material_unique_id = fbx_material->GetUniqueID();
		}
		if (material_count > 0) {
			const int polygon_count{ fbx_mesh->GetPolygonCount() };
			for (int polygon_index = 0; polygon_index < polygon_count; polygon_index++) {
				const int material_index{ fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(polygon_index) };
				subsets.at(material_index).index_count += 3;
			}
			uint32_t offset{ 0 };
			for (mesh::subset& subset : subsets) {
				subset.start_index_location = offset;
				offset += subset.index_count;
				subset.index_count = 0;
			}
		}

		const int polygon_count{ fbx_mesh->GetPolygonCount() };
		mesh.vertices.resize(polygon_count * 3LL);
		mesh.indices.resize(polygon_count * 3LL);

		FbxStringList uv_names;
		fbx_mesh->GetUVSetNames(uv_names);
		const FbxVector4* control_points{ fbx_mesh->GetControlPoints() };
		for (int polygon_index = 0; polygon_index < polygon_count; ++polygon_index) {

			const int material_index{ material_count > 0 ? fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(polygon_index) : 0 };
			mesh::subset& subset{ subsets.at(material_index) };
			const uint32_t offset{ subset.start_index_location + subset.index_count };

			for (int position_in_polygon = 0; position_in_polygon < 3; ++position_in_polygon) {
				const int vertex_index{ polygon_index * 3 + position_in_polygon };

				mesh::vertex vertex;
				const int polygon_vertex{ fbx_mesh->GetPolygonVertex(polygon_index, position_in_polygon) };
				vertex.position.x = static_cast<float>(control_points[polygon_vertex][0]);
				vertex.position.y = static_cast<float>(control_points[polygon_vertex][1]);
				vertex.position.z = static_cast<float>(control_points[polygon_vertex][2]);

				update_maximum(mesh.maximum, vertex.position);
				update_minimum(mesh.minimum, vertex.position);

				const vector<bone_inf>& bones_per_control{ bones.at(polygon_vertex) };

				bone_inf overflow[MAX_BONE_INF];
				bool use_overflow = false;
				if (bones_per_control.size() > MAX_BONE_INF) {
					use_overflow = true;
					for (int i = 0; i < MAX_BONE_INF; i++) { overflow[i] = { 0, 0 }; }
					int min = 0;
					bool update_min = true;
					for (size_t influence_index = 0; influence_index < bones_per_control.size(); ++influence_index) {
						if (influence_index < MAX_BONE_INF) { overflow[influence_index] = bones_per_control.at(influence_index); }
						else {
							if (update_min) {
								for (int i = 0; i < MAX_BONE_INF; i++) {
									if (i == min) { continue; }
									if (overflow[i].weight < overflow[min].weight) { min = i; }
								}
								update_min = false;
							}
							if (bones_per_control.at(influence_index).weight > overflow[min].weight) {
								overflow[min] = bones_per_control.at(influence_index);
								update_min = true;
							}
						}
					}
					float total_weight = 0.0f;
					for (int i = 0; i < MAX_BONE_INF; i++) { total_weight += overflow[i].weight; }
					for (int i = 0; i < MAX_BONE_INF; i++) { overflow[i].weight *= 1.0f / total_weight; }
				}

				size_t max_influence = MAX_BONE_INF < bones_per_control.size() ? MAX_BONE_INF : bones_per_control.size();
				for (size_t influence_index = 0; influence_index < max_influence; ++influence_index) {
					//_ASSERT_EXPR(influence_index < MAX_BONE_INF, L"Max Bone Influences too low, raise or implement overflow.");
					const bone_inf& inf = (use_overflow ? overflow[influence_index] : bones_per_control.at(influence_index));
					vertex.bone_weights[influence_index] = inf.weight;
					vertex.bone_indices[influence_index] = inf.index;
				}

				if (fbx_mesh->GetElementNormalCount() > 0) {
					FbxVector4 normal;
					fbx_mesh->GetPolygonVertexNormal(polygon_index, position_in_polygon, normal);
					vertex.normal.x = static_cast<float>(normal[0]);
					vertex.normal.y = static_cast<float>(normal[1]);
					vertex.normal.z = static_cast<float>(normal[2]);
				}

				if (fbx_mesh->GetElementUVCount() > 0) {
					FbxVector2 uv;
					bool unmapped_uv;
					fbx_mesh->GetPolygonVertexUV(polygon_index, position_in_polygon, uv_names[0], uv, unmapped_uv);
					vertex.texcoord.x = static_cast<float>(uv[0]);
					vertex.texcoord.y = 1.0f - static_cast<float>(uv[1]);
				}

				if (fbx_mesh->GenerateTangentsData(0, false)) {
					const FbxGeometryElementTangent* tangent = fbx_mesh->GetElementTangent();
					vertex.tangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[0]);
					vertex.tangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[1]);
					vertex.tangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[2]);
					vertex.tangent.w = static_cast<float>(tangent->GetDirectArray().GetAt(vertex_index)[3]);
				}

				mesh.vertices.at(vertex_index) = std::move(vertex);
				mesh.indices.at(static_cast<size_t>(offset) + position_in_polygon) = vertex_index;
				subset.index_count++;
			}
		}
	}

	for (auto& mesh : meshes) { update_maximum(maximum, mesh.maximum); update_minimum(minimum, mesh.minimum); }
}

void find_property(const FbxSurfaceMaterial* fbx_material, FbxProperty& fbx_property, vector<string>& attempts) {
	for (const auto& attempt : attempts) {
		fbx_property = fbx_material->FindProperty(attempt);
		if (fbx_property.IsValid()) { return; }
	}
	fbx_property = FbxProperty();
}

void skinned_mesh::fetch_materials(string fbx_filename, FbxScene* fbx_scene) {
	const size_t node_count{ scene_view.nodes.size() };
	const string fbx_filepath = fbx_filename.get_filepath();
	for (size_t node_index = 0; node_index < node_count; node_index++) {
		const scene_view::node& node{ scene_view.nodes.at(node_index) };
		const FbxNode* fbx_node{ fbx_scene->FindNodeByName((const char*)node.name) };

		const int material_count{ fbx_node->GetMaterialCount() };
		for (int material_index = 0; material_index < material_count; material_index++) {
			const FbxSurfaceMaterial* fbx_material{ fbx_node->GetMaterial(material_index) };

			material material;
			material.unique_id	= fbx_material->GetUniqueID();
			material.name		= fbx_material->GetName();

			FbxProperty fbx_property;

			/* Albedo */ {
				vector<string> attempts{ FbxSurfaceMaterial::sDiffuse, "Diffuse", "BaseColor", "baseColor", "DiffuseColor" };
				find_property(fbx_material, fbx_property, attempts);
				if (fbx_property.IsValid()) {
					const FbxDouble3 fbx_albedo{ fbx_property.Get<FbxDouble3>() };
					color albedo;
					albedo.r = static_cast<float>(fbx_albedo[0]);
					albedo.g = static_cast<float>(fbx_albedo[1]);
					albedo.b = static_cast<float>(fbx_albedo[2]);
					albedo.a = 1.0f;

					const FbxFileTexture* fbx_texture{ fbx_property.GetSrcObject<FbxFileTexture>() };
					if (fbx_texture) { 
						const string texture_filename = fbx_texture->GetFileName();
						material.textures[texture_type::texture_map] = std::make_unique<material_texture_file>(texture_filename.get_filename()); 
					}
					else { material.textures[texture_type::texture_map] = std::make_unique<material_texture_dummy>(albedo); }
				}
			}
			
			string ORM_files[3]{""};
			color ORM = DEFAULT_ORM_MAP;

			/* Occlusion */ {
				vector<string> attempts{ "AmbientOcclusion", "AO" };
				find_property(fbx_material, fbx_property, attempts);
				if (fbx_property.IsValid()) {
					if (const FbxFileTexture* tex = fbx_property.GetSrcObject<FbxFileTexture>()) {
						ORM_files[0] = fbx_filepath + tex->GetRelativeFileName();
					}
					else {
						const FbxDouble fbx_occlusion{ fbx_property.Get<FbxDouble>() };
						ORM.r = static_cast<float>(fbx_occlusion);
					}
				}
			}

			/* Roughness */ {
				vector<string> attempts{ "Roughness",};
				find_property(fbx_material, fbx_property, attempts);
				if (fbx_property.IsValid()) {
					const FbxDouble fbx_roughness{ fbx_property.Get<FbxDouble>() };
					ORM.g = static_cast<float>(fbx_roughness);
				}
				else {
					attempts = { FbxSurfaceMaterial::sShininess, FbxSurfaceMaterial::sSpecularFactor, FbxSurfaceMaterial::sReflectionFactor };
					find_property(fbx_material, fbx_property, attempts);
					if (fbx_property.IsValid()) {
						const FbxDouble fbx_shininess{ fbx_property.Get<FbxDouble>() };
						ORM.g = sqrtf(2.0f / (static_cast<float>(fbx_shininess) + 2.0f));
					}
				}
			}

			/* Metallic */ {
				vector<string> attempts{ "Metalness", "Metallic" };
				find_property(fbx_material, fbx_property, attempts);
				if (fbx_property.IsValid()) {
					const FbxDouble fbx_metallic{ fbx_property.Get<FbxDouble>() };
					ORM.b = static_cast<float>(fbx_metallic);
				}
				else {
					attempts = { FbxSurfaceMaterial::sShininess, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sReflection };
					find_property(fbx_material, fbx_property, attempts);
					const FbxDouble3 fbx_specular{ fbx_property.Get<FbxDouble3>() };
					float3 specular;
					specular.x = static_cast<float>(fbx_specular[0]);
					specular.y = static_cast<float>(fbx_specular[1]);
					specular.z = static_cast<float>(fbx_specular[2]);
					ORM.b = clamp(0.0f, (specular.x + specular.y + specular.z) / 3, 1.0f);
				}
			}

			material.textures[texture_type::ORM] = std::make_unique<material_texture_unpacked_orm>(ORM, ORM_files);

			/* Normal */ {
				vector<string> attempts{ FbxSurfaceMaterial::sNormalMap, "NormalMap", "NormalTexture" };
				find_property(fbx_material, fbx_property, attempts);
				if (fbx_property.IsValid()) {
					const FbxFileTexture* fbx_texture{ fbx_property.GetSrcObject<FbxFileTexture>() };
					if (fbx_texture) { 
						const string texture_filename = fbx_texture->GetFileName();
						material.textures[texture_type::normal_map] = std::make_unique<material_texture_file>(texture_filename.get_filename()); 
					}
				}
				else {
					attempts = { FbxSurfaceMaterial::sBump, "Bump" };
					find_property(fbx_material, fbx_property, attempts);
					const FbxFileTexture* fbx_texture{ fbx_property.GetSrcObject<FbxFileTexture>() };
					if (fbx_texture) { 
						const string texture_filename = fbx_texture->GetFileName();
						material.textures[texture_type::normal_map] = std::make_unique<material_texture_height>(texture_filename.get_filename());
					}
				}
			}

			/* Emissive */ {
				vector<string> attempts{ FbxSurfaceMaterial::sEmissive, "Emissive", "EmissiveColor", "Emission" };
				find_property(fbx_material, fbx_property, attempts);
				if (fbx_property.IsValid()) {
					const FbxDouble3 fbx_emissive{ fbx_property.Get<FbxDouble3>() };
					color emissive;
					emissive.r = static_cast<float>(fbx_emissive[0]);
					emissive.g = static_cast<float>(fbx_emissive[1]);
					emissive.b = static_cast<float>(fbx_emissive[2]);

					{
						FbxProperty factor_property;
						vector<string> factor_attempts{ FbxSurfaceMaterial::sEmissiveFactor, "EmissiveFactor", "EmissiveIntensity", "EmissionStrength" };
						if (factor_property.IsValid()) {
							emissive.a = static_cast<float>(factor_property.Get<FbxDouble>());
						}
						else emissive.a = 0.0f;
					}

					const FbxFileTexture* fbx_texture{ fbx_property.GetSrcObject<FbxFileTexture>() };
					if (fbx_texture) { 
						const string texture_filename = fbx_texture->GetFileName();
						material.textures[texture_type::emissive] = std::make_unique<material_texture_file>(texture_filename.get_filename()); 
					}
					else { material.textures[texture_type::emissive] = std::make_unique<material_texture_dummy>(emissive); }
				}
			}

			materials.emplace(material.unique_id, std::move(material));
		}

		if (material_count == 0) { materials.emplace(); }
	}
}

void skinned_mesh::fetch_bones(const FbxMesh* fbx_mesh, vector<vector<bone_inf>>& bones) {
	const int control_count{ fbx_mesh->GetControlPointsCount() };
	bones.resize(control_count);

	const int skin_count{ fbx_mesh->GetDeformerCount() };
	for (int skin_index = 0; skin_index < skin_count; skin_index++) {
		const FbxSkin* fbx_skin{ static_cast<FbxSkin*>(fbx_mesh->GetDeformer(skin_index, FbxDeformer::eSkin)) };

		const int cluster_count{ fbx_skin->GetClusterCount() };
		for (int cluster_index = 0; cluster_index < cluster_count; cluster_index++) {
			const FbxCluster* fbx_cluster{ fbx_skin->GetCluster(cluster_index) };

			const int control_indices_count{ fbx_cluster->GetControlPointIndicesCount() };
			for (int control_indices_index = 0; control_indices_index < control_indices_count; control_indices_index++) {
				int		control_index{ fbx_cluster->GetControlPointIndices()[control_indices_index] };
				double	control_weight{ fbx_cluster->GetControlPointWeights()[control_indices_index] };
				bone_inf& bone_influence{ bones.at(control_index).emplace_back() };
				bone_influence.index = static_cast<uint32_t>(cluster_index);
				bone_influence.weight = static_cast<float>(control_weight);
			}
		}
	}
}

void skinned_mesh::fetch_skeleton(const FbxMesh* fbx_mesh, skeleton& bind_pose) {
	const int deformer_count = fbx_mesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int deformer_index = 0; deformer_index < deformer_count; deformer_index++) {
		FbxSkin* skin = static_cast<FbxSkin*>(fbx_mesh->GetDeformer(deformer_index, FbxDeformer::eSkin));
		const int cluster_count = skin->GetClusterCount();
		bind_pose.bones.resize(cluster_count);
		for (int cluster_index = 0; cluster_index < cluster_count; cluster_index++) {
			FbxCluster* cluster = skin->GetCluster(cluster_index);

			skeleton::bone& bone{ bind_pose.bones.at(cluster_index) };
			bone.name = cluster->GetLink()->GetName();
			bone.unique_id = cluster->GetLink()->GetUniqueID();
			bone.parent_index = bind_pose.index_of(cluster->GetLink()->GetParent()->GetUniqueID());
			bone.node_index = scene_view.index_of(bone.unique_id);

			FbxAMatrix global_reference;
			cluster->GetTransformMatrix(global_reference);

			FbxAMatrix cluster_reference;
			cluster->GetTransformLinkMatrix(cluster_reference);

			bone.offset_transform = to_xmfloat4x4(cluster_reference.Inverse() * global_reference);
		}
	}
	//bind_pose.compute_all_globals();
}

void skinned_mesh::fetch_attachment(FbxNode* fbx_node, skeleton& bind_pose) {
	for (FbxNode* parent = fbx_node->GetParent(); parent; parent = parent->GetParent()) {
		FbxNodeAttribute* attr = parent->GetNodeAttribute();
		if (!attr) continue;
		if (attr->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
			skeleton::bone& bone{ bind_pose.bones.emplace_back() };
			bone.name = parent->GetName();
			bone.unique_id = parent->GetUniqueID();
			bone.parent_index = bind_pose.index_of(parent->GetParent()->GetUniqueID());
			bone.node_index = scene_view.index_of(bone.unique_id);

			FbxAMatrix mesh_global = fbx_node->EvaluateGlobalTransform();
			FbxAMatrix parent_global = parent->EvaluateGlobalTransform();

			bone.offset_transform = to_xmfloat4x4(parent_global.Inverse() * mesh_global);
			break;
		}
	}
}

void skinned_mesh::fallback_skeleton(skeleton& bind_pose) {
	skeleton::bone& bone{ bind_pose.bones.emplace_back() };
	bone.name = "default_bone";
	bone.unique_id = 0;
	bone.parent_index = -1;
	bone.node_index = 0;//scene_view.index_of(0);
	bone.offset_transform = matrix_id;
}

void skinned_mesh::fetch_animations(FbxScene* fbx_scene, std::unordered_map<string, animation>& animation_clips, float sampling_rate) {
	FbxArray<FbxString*> animation_stack_names;
	fbx_scene->FillAnimStackNameArray(animation_stack_names);
	const int animation_stack_count{ animation_stack_names.GetCount() };
	for (int animation_stack_index = 0; animation_stack_index < animation_stack_count; animation_stack_index++) {
		string name = animation_stack_names[animation_stack_index]->Buffer();
		auto emplace_attempt = animation_clips.try_emplace(name);
		if (!emplace_attempt.second) { // Duplicate animation
			delete animation_stack_names[animation_stack_index];
			continue; 
		}
		animation& animation_clip{ emplace_attempt.first->second };
		animation_clip.name = name;

		FbxAnimStack* animation_stack{ fbx_scene->FindMember<FbxAnimStack>(animation_clip.name) };
		fbx_scene->SetCurrentAnimationStack(animation_stack);

		const FbxTime::EMode time_mode{ fbx_scene->GetGlobalSettings().GetTimeMode() };
		FbxTime one_second;
		one_second.SetTime(0, 0, 1, 0, 0, time_mode);
		animation_clip.sampling_rate = (sampling_rate > 0 ? sampling_rate : static_cast<float>(one_second.GetFrameRate(time_mode)));
		const FbxTime sampling_interval{ static_cast<FbxLongLong>(one_second.Get() / animation_clip.sampling_rate) };
		const FbxTakeInfo* take_info{ fbx_scene->GetTakeInfo((const char*)animation_clip.name) };
		const FbxTime start_time{ take_info->mLocalTimeSpan.GetStart() };
		const FbxTime stop_time{ take_info->mLocalTimeSpan.GetStop() };

		for (FbxTime time = start_time; time < stop_time; time += sampling_interval) {
			animation::keyframe& keyframe{ animation_clip.sequence.emplace_back() };

			const size_t node_count{ scene_view.nodes.size() };
			keyframe.nodes.resize(node_count);
			for (size_t node_index = 0; node_index < node_count; node_index++) {
				FbxNode* fbx_node{ fbx_scene->FindNodeByName((const char*)scene_view.nodes.at(node_index).name) };

				if (fbx_node) {
					animation::keyframe::node& node{ keyframe.nodes.at(node_index) };
					node.global_transform = to_xmfloat4x4(fbx_node->EvaluateGlobalTransform(time));
					const FbxAMatrix& local_transform{ fbx_node->EvaluateLocalTransform(time) };
					node.local_transform.set_scl(to_xmfloat3(local_transform.GetS()));
					node.local_transform.set_qtn(to_xmfloat4(local_transform.GetQ()));
					node.local_transform.set_pos(to_xmfloat3(local_transform.GetT()));
				}
			}
		}
		delete animation_stack_names[animation_stack_index];
	}
}

// Com Objects

void skinned_mesh::create_com_objects() {
	for (mesh& mesh : meshes) {
		mesh.create_buffers();
#if 0
		mesh.vertices.clear();
		mesh.indices.clear();
#endif
	}

	update_meshes(true);

	for (auto& mat : materials) { mat.second.construct(); }

	input_element_desc = {
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TANGENT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TEXCOORD",	1, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TEXCOORD",	2, DXGI_FORMAT_R32G32B32A32_UINT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	create_shaders("skinned_full");
}

// Public 

bool skinned_mesh::append_animations(string animation_filename, float sampling_rate) {
	string animation_filepath = animation_filename.get_filepath();
	string cereal_filename = animation_filename.get_filename().replace_ext("cereal");
	string full_cereal_filename = animation_filepath + fbx_name + cereal_filename;
	std::unordered_map<string, animation> new_animations;
	if (full_cereal_filename.file_exists()) {
		UNCEREAL(full_cereal_filename, new_animations);
	}
	else {
		FbxManager* fbx_manager{ FbxManager::Create() };
		FbxScene* fbx_scene{ FbxScene::Create(fbx_manager, "") };

		FbxImporter* fbx_importer{ FbxImporter::Create(fbx_manager, "") };
		bool import_status{ false };
		import_status = fbx_importer->Initialize(animation_filename);
		_ASSERT_EXPR_A(import_status, fbx_importer->GetStatus().GetErrorString());
		import_status = fbx_importer->Import(fbx_scene);
		_ASSERT_EXPR_A(import_status, fbx_importer->GetStatus().GetErrorString());

		fetch_animations(fbx_scene, new_animations, sampling_rate);

		fbx_manager->Destroy();
		CEREAL(full_cereal_filename, new_animations);
	}
	animations.merge(new_animations);
	return true;
}

void skinned_mesh::update_animation(animation::keyframe& keyframe) {
	const size_t node_count{ keyframe.nodes.size() };
	for (size_t node_index = 0; node_index < node_count; ++node_index) {
		animation::keyframe::node& node{ keyframe.nodes.at(node_index) };

		int64_t parent_index{ scene_view.nodes.at(node_index).parent_index };
		DirectX::XMMATRIX P{ parent_index < 0 ? DirectX::XMMatrixIdentity() : DirectX::XMLoadFloat4x4(&keyframe.nodes.at(parent_index).global_transform) };

		DirectX::XMStoreFloat4x4(&node.global_transform, node.local_transform * P);
	}
}

uint32_t skinned_mesh::ray_collision(const transform& model_transform, const float3& origin, const float3& ray, float3* out_int_point, float3* out_int_normal, bool any_hit) const {
	uint32_t hit = 0;
	float3 int_point, int_normal;
	for (auto& mesh : meshes) {
		if (mesh.ray_collision(model_transform, origin, ray, out_int_point ? &int_point : nullptr, out_int_normal ? &int_normal : nullptr, any_hit)) {
			if (any_hit) { return 1; }
			if (out_int_point && out_int_normal) {
				if ((hit == 0) || (int_point - origin).mag_sq() < (*out_int_point - origin).mag_sq()) {
					*out_int_point = int_point;
					*out_int_normal = int_normal;
				}
			}
			hit++;
		}
	}
	return hit;
}

void skinned_mesh::render(const float4x4& world, const color& material_color) const {
	device::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	constants data{};
	data.material_color = material_color;
#ifdef SKIN_CPU
	DirectX::XMStoreFloat4x4(&data.world, DirectX::XMLoadFloat4x4(&world) * DirectX::XMLoadFloat4x4(&coordinate_system_transforms[coord_sys]));
	device::context()->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
#else
	if (is_animating()) {
		DirectX::XMStoreFloat4x4(&data.world, DirectX::XMLoadFloat4x4(&world) * DirectX::XMLoadFloat4x4(&coordinate_system_transforms[coord_sys]));
		device::context()->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
	}
#endif
	device::context()->VSSetConstantBuffers(FULL_VS_CB, 1, constant_buffer.GetAddressOf());
	uint32_t stride{ sizeof(mesh::vertex) };
	uint32_t offset{ 0 };

	for (const mesh& mesh : meshes) {

		device::context()->IASetVertexBuffers(0, 1, mesh.get_vertices(), &stride, &offset);
		device::context()->IASetIndexBuffer(mesh.get_indices(), DXGI_FORMAT_R32_UINT, 0);

#ifdef SKIN_GPU
		device::context()->VSSetConstantBuffers(BONE_CB, 1, mesh.get_bone_buffer());
		if (!is_animating()) {
			DirectX::XMStoreFloat4x4(&data.world, DirectX::XMLoadFloat4x4(&mesh.default_global_transform) * DirectX::XMLoadFloat4x4(&world) * DirectX::XMLoadFloat4x4(&coordinate_system_transforms[coord_sys]));
			device::context()->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
		}
#endif

		for (const mesh::subset& subset : mesh.subsets)
		{
			const material& material{ materials.at(subset.material_unique_id) };
			material.bind(0);
			device::context()->DrawIndexed(subset.index_count, subset.start_index_location, 0);
		}
	}
}