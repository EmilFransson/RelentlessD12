#pragma once
#include "Assets/AssetMeta.h"
#include "ECSCommon.h"
#include "Math/MathTypes.h"
#include "Utility/Common.h"

namespace Relentless
{
	struct TransformComponent
	{
		Transform WorldTransform;
		Transform LocalTransform;
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
		Color Color = Colors::White;
		float Intensity = 1.0f;
	};

	struct PointLightComponent
	{
		explicit PointLightComponent(const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f))
			:Intensity{ 1.0f },
			 Color{ color }
		{}

		float Intensity;
		DirectX::XMFLOAT3 Color;
	};

	struct CameraComponent
	{
		Matrix WorldToView = Matrix::Identity;
		Matrix ViewToClip = Matrix::Identity;
		float FieldOfViewDegrees = 60.0f;
		float ClippingPlaneNear = 0.1f;
		float ClippingPlaneFar = 1000.0f;
		bool IsMainCamera = false;
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

	struct RotatorComponent
	{
		//ID
	};
}