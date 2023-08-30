#include "SceneSerializer.h"
#include "../Utility/SerializeUtilities.h"
#pragma warning(push, 0)
#define YAML_CPP_STATIC_DEFINE 1
#include "yaml-cpp/yaml.h"
#pragma warning(pop)

namespace Relentless
{
	static void SerializeEntity(YAML::Emitter& out, EntityManager& manager, entity entity) noexcept
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << manager.Get<IDComponent>(entity).UuId;
		
		if (manager.Has<NameComponent>(entity))
		{
			out << YAML::Key << "NameComponent";
			out << YAML::BeginMap;
		
			out << YAML::Key << "Name" << YAML::Value << manager.Get<NameComponent>(entity).Name;
		
			out << YAML::EndMap;
		}
		if (manager.Has<TransformComponent>(entity))
		{
			auto& tc = manager.Get<TransformComponent>(entity);
		
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;
		
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
		
			out << YAML::EndMap;
		}
		if (manager.Has<MeshFilterComponent>(entity))
		{
			auto& mfc = manager.Get<MeshFilterComponent>(entity);
		
			out << YAML::Key << "MeshFilterComponent";
			out << YAML::BeginMap;
		
			out << YAML::Key << "MeshHandle" << YAML::Value << mfc.MeshHandle.UUID;
		
			out << YAML::EndMap;
		}
		if (manager.Has<MeshRendererComponent>(entity))
		{
			out << YAML::Key << "MeshRendererComponent";
			out << YAML::BeginMap;
		
			out << YAML::Key << "MaterialHandle" << YAML::Value << manager.Get<MeshRendererComponent>(entity).MaterialHandle.UUID;
		
			out << YAML::EndMap;
		}
		if (manager.Has<DirectionalLightComponent>(entity))
		{
			auto& dlc = manager.Get<DirectionalLightComponent>(entity);
		
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap;
		
			out << YAML::Key << "Direction" << dlc.Direction;
			out << YAML::Key << "Intensity" << dlc.Intensity;
			out << YAML::Key << "Color" << dlc.Color;
		
			out << YAML::EndMap;
		}
		if (manager.Has<PointLightComponent>(entity))
		{
			auto& plc = manager.Get<PointLightComponent>(entity);
		
			out << YAML::Key << "PointLightComponent";
			out << YAML::BeginMap;
		
			out << YAML::Key << "Position" << plc.Position;
			out << YAML::Key << "Intensity" << plc.Intensity;
			out << YAML::Key << "Color" << plc.Color;
		
			out << YAML::EndMap;
		}
		if (manager.Has<OpaquePassComponent>(entity))
		{
			out << YAML::Key << "OpaquePassComponent";
			out << YAML::BeginMap;
		
			out << YAML::Key << "ForwardPass" << YAML::Value << true;
		
			out << YAML::EndMap;
		}
		if (manager.Has<RootComponent>(entity))
		{
			out << YAML::Key << "RootComponent";
			out << YAML::BeginMap;
		
			out << YAML::Key << "Root" << YAML::Value << true;
		
			out << YAML::EndMap;
		}
		if (manager.Has<IsChildComponent>(entity))
		{
			auto& icc = manager.Get<IsChildComponent>(entity);
		
			out << YAML::Key << "IsChildComponent";
			out << YAML::BeginMap;
		
			out << YAML::Key << "Parent" << YAML::Value << manager.Get<IDComponent>(icc.Parent).UuId;
			out << YAML::Key << "LocalTranslation" << YAML::Value << icc.LocalTranslation;
			out << YAML::Key << "LocalRotation" << YAML::Value << icc.LocalRotation;
			out << YAML::Key << "LocalScale" << YAML::Value << icc.LocalScale;
		
			out << YAML::EndMap;
		}
		if (manager.Has<ParentComponent>(entity))
		{
			auto& pc = manager.Get<ParentComponent>(entity);
		
			out << YAML::Key << "ParentComponent";
			out << YAML::BeginMap;
		
			out << YAML::Key << "Children";
			out << YAML::Flow;
		
			out << YAML::BeginSeq;
		
			for (uint32_t i{ 0u }; i < pc.Children.size(); ++i)
			{
				out << manager.Get<IDComponent>(pc.Children[i]).UuId;
			}
		
			out << YAML::EndSeq;
			
			out << YAML::EndMap;
		}
		
		out << YAML::EndMap;
	}

	void SceneSerializer::Serialize(const std::shared_ptr<Scene>& pScene, const std::string& filepath) noexcept
	{
		RLS_ASSERT(pScene, "Scene is invalid");
		
		YAML::Emitter out;
		out << YAML::BeginMap;
		
		out << YAML::Key << "Scene" << YAML::Value << pScene->GetName();
		
		out << YAML::Key << "Entities" << YAML::BeginSeq;
		pScene->GetEntityManager().Collect<TransformComponent>().Do([&](entity e)
			{
				SerializeEntity(out, pScene->GetEntityManager(), e);
			});
		
		out << YAML::EndSeq;
		out << YAML::EndMap;
		
		std::ofstream outFile(filepath);
		outFile << out.c_str();
		outFile.close();
	}

	bool SceneSerializer::Deserialize(const std::shared_ptr<Scene>& pScene, const std::string& filepath) noexcept
	{
		RLS_ASSERT(pScene, "Scene is invalid.");
		
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		stream.close();
		
		YAML::Node data = YAML::Load(strStream.str());
		
		if (!data["Scene"])
			return false;
		
		auto& mgr = pScene->GetEntityManager();
		
		std::string sceneName = data["Scene"].as<std::string>();
		RLS_CORE_TRACE("Deserializing scene {0}", sceneName);
		
		auto entities = data["Entities"];
		for (auto entity : entities)
		{
			std::string uuidAsString = entity["Entity"].as<std::string>();
			std::wstring uuidAsWString = ConvertStringToWstring(uuidAsString);
			GUID uuid;
			IIDFromString(uuidAsWString.c_str(), &uuid);
		
			std::string name;
			auto nameComponent = entity["NameComponent"];
			if (nameComponent)
			{
				name = nameComponent["Name"].as<std::string>();
			}
		
			RLS_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuidAsString, name);
		
			RLS::entity deserializedEntity;
			if (entity["DirectionalLightComponent"])
			{
				deserializedEntity = pScene->CreateLight(name.c_str(), LightType::Directional);
				mgr.Get<IDComponent>(deserializedEntity).UuId = uuid;
			}
			else if (entity["PointLightComponent"])
			{
				deserializedEntity = pScene->CreateLight(name.c_str(), LightType::Point);
				mgr.Get<IDComponent>(deserializedEntity).UuId = uuid;
			}
			else
			{
				//Create "default" entity:
				deserializedEntity = pScene->CreateEntityWithUUID(name.c_str(), uuid);
			}
		
			auto transformComponent = entity["TransformComponent"];
			if (transformComponent)
			{
				auto& tc = mgr.Get<TransformComponent>(deserializedEntity);
				tc.Translation = transformComponent["Translation"].as<DirectX::XMFLOAT3>();
				tc.Rotation = transformComponent["Rotation"].as<DirectX::XMFLOAT3>();
				tc.Scale = transformComponent["Scale"].as<DirectX::XMFLOAT3>();
			}
		
			auto meshFilterComponent = entity["MeshFilterComponent"];
			if (meshFilterComponent)
			{
				auto& mfc = mgr.Add<MeshFilterComponent>(deserializedEntity);
				std::wstring vertexBufferIDAsString = ConvertStringToWstring(meshFilterComponent["MeshHandle"].as<std::string>());
				MeshHandle meshHandle;
				IIDFromString(vertexBufferIDAsString.c_str(), &meshHandle.UUID);
				
				mfc.MeshHandle = meshHandle;
			}
		
			auto meshRendererComponent = entity["MeshRendererComponent"];
			if (meshRendererComponent)
			{
				auto& mrc = mgr.Add<MeshRendererComponent>(deserializedEntity);
				std::wstring materialHandleIDAsString = ConvertStringToWstring(meshRendererComponent["MaterialHandle"].as<std::string>());
				MaterialHandle materialHandle;
				IIDFromString(materialHandleIDAsString.c_str(), &materialHandle.UUID);
				
				mrc.MaterialHandle = materialHandle;
			}
		
			auto opaquePassComponent = entity["OpaquePassComponent"];
			if (opaquePassComponent)
			{
				mgr.Add<OpaquePassComponent>(deserializedEntity);
			}
		
			auto directionalLightComponent = entity["DirectionalLightComponent"];
			if (directionalLightComponent)
			{
				//If an entity is discovered to be a directional light it will be created with the
				//component earlier (see above)
				auto& dlc = mgr.Get<DirectionalLightComponent>(deserializedEntity);
				dlc.Direction = directionalLightComponent["Direction"].as<DirectX::XMFLOAT3>();
				dlc.Intensity = directionalLightComponent["Intensity"].as<float>();
				dlc.Color = directionalLightComponent["Color"].as<DirectX::XMFLOAT3>();
			}
		
			auto pointLightComponent = entity["PointLightComponent"];
			if (pointLightComponent)
			{
				//If an entity is discovered to be a point light it will be created with the
				//component earlier (see above)
				auto& plc = mgr.Get<PointLightComponent>(deserializedEntity);
				plc.Position = pointLightComponent["Position"].as<DirectX::XMFLOAT3>();
				plc.Intensity = pointLightComponent["Intensity"].as<float>();
				plc.Color = pointLightComponent["Color"].as<DirectX::XMFLOAT3>();
			}
		}
		
		//Step two: check if entities are parented,
		//now that we are certain that all are loaded.
		for (auto entity : entities)
		{
			std::string uuidAsString = entity["Entity"].as<std::string>();
			std::wstring uuidAsWString = ConvertStringToWstring(uuidAsString);
			GUID uuid1;
			IIDFromString(uuidAsWString.c_str(), &uuid1);
		
			RLS::entity deserializedEntity = pScene->FindEntityByUUID(uuid1);
			RLS_ASSERT(deserializedEntity != NULL_ENTITY, "Deserialized entity was not not found by UUID.");
		
			//auto isChildComponent = entity["IsChildComponent"];
			//if (isChildComponent)
			//{
			//	std::wstring parentEntityIDAsString = ConvertStringToWstring(isChildComponent["Parent"].as<std::string>());
			//	GUID uuid2;
			//	IIDFromString(parentEntityIDAsString.c_str(), &uuid2);
			//
			//	auto& icc = mgr.Add<IsChildComponent>(deserializedEntity);
			//	icc.Parent = pScene->FindEntityByUUID(uuid2);
			//	RLS_ASSERT(icc.Parent != NULL_ENTITY, "Parent entity was not not found by UUID.");
			//
			//	icc.LocalTranslation = isChildComponent["LocalTranslation"].as<DirectX::XMFLOAT3>();
			//	icc.LocalRotation = isChildComponent["LocalRotation"].as<DirectX::XMFLOAT3>();
			//	icc.LocalScale = isChildComponent["LocalScale"].as<DirectX::XMFLOAT3>();
			//}
			
			auto parentComponent = entity["ParentComponent"];
			if (parentComponent)
			{
				std::vector<std::string> childrenIDStrings = parentComponent["Children"].as<std::vector<std::string>>();
				for (auto& idString : childrenIDStrings)
				{
					std::wstring wideIDString = ConvertStringToWstring(idString);
					GUID uuid;
					IIDFromString(wideIDString.c_str(), &uuid);
					auto child = pScene->FindEntityByUUID(uuid);
					pScene->ParentEntity(child, deserializedEntity);
				}
			}
		}

		return true;
	}
}
