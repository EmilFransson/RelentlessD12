#pragma once
#include "Assets/AssetMeta.h"

#include "Core/DLLExport.h"
#include "Core/Folder.h"

#include "ECSCommon.h"

#include "Math/MathTypes.h"

#include "Utility/Common.h"

namespace Relentless
{
	class Scene;

	struct RLS_API TransformComponent
	{
		void AddWorldOffset(const Vector3& aDeltaLocation) noexcept;
		void AddWorldRotation(const Quaternion& aDeltaRotation) noexcept;
		void AddWorldRotationEulerDegrees(const Vector3& aDeltaEulerDegrees) noexcept;
		void AddWorldScale(const Vector3& aDeltaScale) noexcept;
		void AddLocalOffset(const Vector3& aDeltaLocation) noexcept;
		void AddLocalRotation(const Quaternion& aDeltaRotation) noexcept;
		void AddLocalRotationEulerDegrees(const Vector3& aDeltaEulerDegrees) noexcept;
		void AddLocalScale(const Vector3& aDeltaScale) noexcept;

		NO_DISCARD Vector3 GetWorldForward() const noexcept;
		NO_DISCARD const Matrix& GetWorldMatrix() const noexcept;
		NO_DISCARD Vector3 GetWorldLocation() const noexcept;
		NO_DISCARD Quaternion GetWorldRotation() const noexcept;
		NO_DISCARD Vector3 GetWorldRotationEulerDegrees() const noexcept;
		NO_DISCARD Vector3 GetWorldRight() const noexcept;
		NO_DISCARD Vector3 GetWorldScale() const noexcept;
		NO_DISCARD Vector3 GetWorldUp() const noexcept;
		NO_DISCARD Matrix GetLocalMatrix() const noexcept;
		NO_DISCARD const Vector3& GetLocalLocation() const noexcept;
		NO_DISCARD const Quaternion& GetLocalRotation() const noexcept;
		NO_DISCARD Vector3 GetLocalRotationEulerDegrees() const noexcept;
		NO_DISCARD const Vector3& GetLocalScale() const noexcept;

		void SetLocalLocation(const Vector3& aLocation) noexcept;
		void SetLocalRotation(const Quaternion& aRotation) noexcept;
		void SetLocalRotationEulerDegrees(const Vector3& aEulerDegrees) noexcept;
		void SetLocalScale(const Vector3& aScale) noexcept;
		void SetWorldLocation(const Vector3& aLocation) noexcept;
		void SetWorldRotation(const Quaternion& aRotation) noexcept;
		void SetWorldRotationEulerDegrees(const Vector3& aEulerDegrees) noexcept;
		void SetWorldScale(const Vector3& aScale) noexcept;
	private:
		void EnsureWorldUpToDate() const noexcept;
		inline Matrix BuildLocalMatrix(const Transform& aLocalTransform) const noexcept;
		inline Matrix ComposeWorld(const Matrix& aParentWorldMatrix, const Transform& aLocalTransform) const noexcept;
	private:
		friend class Scene;

		Transform LocalTransform;
		mutable Matrix WorldMatrix = Matrix::Identity;
		uint32         LocalVersion = 0u;  
		mutable uint32 WorldVersion = 0u; 
		mutable uint32 LocalVersionSeenForWorld = 0u; 
		mutable uint32 ParentWorldVersionSeen = 0u; 

		Scene* Scene = nullptr;
		entity Self = NULL_ENTITY;
	};

	struct NameComponent
	{
		explicit NameComponent(const char* name)
			: Name{name}
		{ }

		String Name;
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

		UUID UuId = NULL_UUID;
	};

	struct RLS_API LightBaseComponent
	{
	public:
		NO_DISCARD const Color& GetColor() const noexcept;
		NO_DISCARD float GetIntensity() const noexcept;
		NO_DISCARD float GetTemperature() const noexcept;
		NO_DISCARD bool IsUsingTemperature() const noexcept;

		void SetColor(const Color& aColor) noexcept;
		void SetTemperature(float aTemperature) noexcept;
		void SetUseTemperature(bool aUseTemperature) noexcept;

	protected:
		Color m_Color = Colors::Normalize(255.0f, 244.0f, 214.0f, 255.0f);
		float Intensity = 8.0f;
		float Temperature = 6'500.0f;
		bool UseTemperature = false;
	};

	struct DirectionalLightComponent : public LightBaseComponent
	{
		void SetIntensity(float aWatts, float aEfficiency) noexcept
		{
			SetIntensityLux(aWatts * 683.0f * aEfficiency);
		}

		void SetIntensityLux(float aLuxValue) noexcept
		{
			Intensity = aLuxValue;
		}
	};

	struct PointLightComponent : public LightBaseComponent
	{
		NO_DISCARD float GetAttenuationRadius() const noexcept
		{
			return AttenuationRadius;
		}

		void SetAttenuationRadius(float aRadius) noexcept
		{
			AttenuationRadius = aRadius;
		}

		void SetIntensity(float aWatts, float aEfficiency) noexcept
		{
			SetIntensityLumen(aWatts * 683.0f * aEfficiency);
		}

		void SetIntensityCandela(float aCandelaValue) noexcept
		{
			Intensity = aCandelaValue;
		}

		void SetIntensityLumen(float aLumenValue) noexcept
		{
			Intensity = Math::Photometry::LumenToCandela_Point(aLumenValue);
		}
	
	private:
		float AttenuationRadius = 10.0f;
	};

	struct SpotLightComponent : public LightBaseComponent
	{
		NO_DISCARD float GetAttenuationRadius() const noexcept
		{
			return AttenuationRadius;
		}

		NO_DISCARD float GetInnerConeAngleDegrees() const noexcept
		{
			return Math::RadToDeg(InnerConeAngle);
		}

		NO_DISCARD float GetInnerConeAngleRadians() const noexcept
		{
			return InnerConeAngle;
		}

		NO_DISCARD float GetOuterConeAngleDegrees() const noexcept
		{
			return Math::RadToDeg(OuterConeAngle);
		}

		NO_DISCARD float GetOuterConeAngleRadians() const noexcept
		{
			return OuterConeAngle;
		}

		void SetAttenuationRadius(float aRadius) noexcept
		{
			AttenuationRadius = aRadius;
		}

		void SetIntensity(float aWatts, float aEfficiency) noexcept
		{
			SetIntensityLumen(aWatts * 683.0f * aEfficiency);
		}

		void SetIntensityCandela(float aCandelaValue) noexcept
		{
			Intensity = aCandelaValue;
		}

		void SetIntensityLumen(float aLumenValue) noexcept
		{
			Intensity = Math::Photometry::LumenToCandela_Spot(aLumenValue, OuterConeAngle);
		}

		void SetInnerConeAngleDegrees(float aAngleDegrees) noexcept
		{
			InnerConeAngle = Math::DegToRad(aAngleDegrees);
			if (InnerConeAngle > OuterConeAngle)
				std::swap(InnerConeAngle, OuterConeAngle);
		}

		void SetOuterConeAngleDegrees(float aAngleDegrees) noexcept
		{
			OuterConeAngle = Math::DegToRad(aAngleDegrees);
			if (OuterConeAngle < InnerConeAngle)
				std::swap(OuterConeAngle, InnerConeAngle);
		}

	private:
		float AttenuationRadius = 10.0f;
		float InnerConeAngle	= 0.0f;
		float OuterConeAngle	= Math::DegToRad(44.0f);
	};

	struct CameraComponent
	{
		Matrix WorldToView			= Matrix::Identity;
		Matrix ViewToClip			= Matrix::Identity;
		float FieldOfViewDegrees	= 60.0f;
		float ClippingPlaneNear		= 0.1f;
		float ClippingPlaneFar		= 1000.0f;
		bool IsMainCamera			= false;
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

	struct FolderComponent
	{
		Folder Folder;
	};
}