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
		Vector3 Color = Vector3::One;
		float Intensity = 1.0f;
		float Range = 10.0f;
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