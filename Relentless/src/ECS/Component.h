#pragma once
#include "Assets/AssetMeta.h"
#include "ECSCommon.h"
#include "Graphics/D3D12Core.h"
#include "Graphics/GPUTaskManager.h"
#include "Graphics/Resources/Material.h"
#include "Mesh/Mesh.h"
#include "Core/Application.h"
#include "Math/MathTypes.h"

namespace Relentless
{
	struct TransformComponent
	{
		Transform WorldTransform;
		Transform LocalTransform;
	};

	struct DirtyTransformComponent
	{
		DirtyTransformComponent()
			: Updates{ GPUTaskManager::FRAMES_IN_FLIGHT },
			  AdjustedWorldSpace{true},
			  OnlyUpload{false}
		{}

		uint32_t Updates;
		bool AdjustedWorldSpace;
		bool OnlyUpload;
	};

	struct NameComponent
	{
		explicit NameComponent(const char* name)
			: Name{name}
		{ }

		std::string Name;
	};

	struct MeshFilterComponent
	{
		MeshFilterComponent()
			: AssetHandle{ NULL_HANDLE }
		{}

		AssetHandle AssetHandle;
	};

	struct MeshRendererComponent
	{
		MeshRendererComponent()
			: AssetHandle{ NULL_HANDLE }
		{}

		AssetHandle AssetHandle;
	};

	struct DirtyMeshRendererComponent
	{
		DirtyMeshRendererComponent()
			: Updates{ GPUTaskManager::FRAMES_IN_FLIGHT }
		{}

		uint32_t Updates;
	};

	struct OpaquePassComponent
	{
		//ID
	};

	struct IDComponent
	{
		IDComponent()
		{
			UuId = CreateUUID();
		}
		
		IDComponent(const UUID& id)
			: UuId{id}
		{

		}

		UUID UuId;
	};

	struct DirectionalLightComponent
	{
		explicit DirectionalLightComponent(const Vector3& color = Vector3(1.0f, 1.0f, 1.0f))
			: Color{ color },
			  Direction{ 0.0f, 0.0f, 0.0f },
			  Intensity{1.0f}
		{}

		Vector3 Color;
		Vector3 Direction;
		float Intensity;
	};

	struct PointLightComponent
	{
		explicit PointLightComponent(const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f))
			:Position{0.0f, 0.0f, 0.0f},
			 Intensity{ 1.0f },
			 Color{ color }
		{}

		DirectX::XMFLOAT3 Position;
		float Intensity;
		DirectX::XMFLOAT3 Color;
	};

	struct CameraComponent
	{
		CameraComponent()
			: ViewMatrix{},
			  ProjectionMatrix{},
			  FieldOfViewDegrees{60.0f},
			  ClippingPlaneNear{0.3f},
			  ClippingPlaneFar{1000.0f},
			  ClearColor{ DirectX::XMFLOAT4(DirectX::Colors::CornflowerBlue) },
			  IsMainCamera{false}
		{}
		DirectX::XMFLOAT4X4 ViewMatrix;
		DirectX::XMFLOAT4X4 ProjectionMatrix;
		float FieldOfViewDegrees;
		float ClippingPlaneNear;
		float ClippingPlaneFar;
		DirectX::XMFLOAT4 ClearColor;
		bool IsMainCamera;
	};

	struct RootComponent
	{
		//ID
	};

	struct ParentComponent
	{
		std::vector<entity> Children;
	};

	struct IsChildComponent
	{
		explicit IsChildComponent() noexcept
			: Parent{ NULL_ENTITY }
		{} 

		entity Parent;
	};

	struct SelectedInEditorComponent
	{
		//ID
	};

	struct HiddenInGameComponent
	{
		//ID
	};
}