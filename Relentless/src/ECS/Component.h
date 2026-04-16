#pragma once
#include "Assets/AssetMeta.h"

#include "Callback/CoreBroadcasters.h"
#include "Core/DLLExport.h"
#include "Core/Folder.h"

#include "ECSCommon.h"

#include "Graphics/RHI/Texture.h"

#include "Math/MathTypes.h"

#include <StaticTypeInfo/type_index.h>

#include "Utility/Common.h"

namespace Relentless
{
	class EntityManager;
	class Environment;
	class Scene;

	struct RLS_API IComponent
	{
		virtual const TypeIndex& GetStaticType() const noexcept = 0;
	};

	template<typename ComponentType>
	struct RLS_API ComponentBase : public IComponent
	{
		virtual const TypeIndex& GetStaticType() const noexcept override final
		{
			return StaticType();
		}

		static constexpr const TypeIndex& StaticType()
		{
			static constexpr TypeIndex typeIndex = getTypeIndex<ComponentType>();
			return typeIndex;
		}

	protected:
		void BroadcastPropertyChanged(uint64 aProperty) noexcept
		{
			CoreObjectBroadcasters::OnComponentPropertyChanged(EditorSelf, ComponentType::StaticType(), this, aProperty);
		}

	private:
		friend class EntityManager;
		entity EditorSelf = NULL_ENTITY;
	};

	template<typename ComponentType>
	struct RLS_API ManagedComponent : public ComponentBase<ComponentType>
	{
		ManagedComponent(ManagedComponent&&) noexcept = default;
		ManagedComponent& operator=(ManagedComponent&&) noexcept = default;

		ManagedComponent(const ManagedComponent& aOtherComponent) = delete;
		ManagedComponent& operator=(const ManagedComponent& aOtherComponent) = delete;

		virtual void CopyFrom(const ComponentType& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) = 0;
	protected:
		ManagedComponent() = default;
		virtual ~ManagedComponent() = default;

		virtual void OnBound() noexcept {}
	protected:
		friend class EntityManager;

		entity m_Self = NULL_ENTITY;
		EntityManager* m_EntityManager = nullptr;
	};

	struct RLS_API TransformComponent : public ManagedComponent<TransformComponent>
	{
	public:
		struct DirtyRenderState {};

		void AddWorldOffset(const Vector3& aDeltaLocation) noexcept;
		void AddWorldRotation(const Quaternion& aDeltaRotation) noexcept;
		void AddWorldRotationEulerDegrees(const Vector3& aDeltaEulerDegrees) noexcept;
		void AddWorldScale(const Vector3& aDeltaScale) noexcept;
		void AddLocalOffset(const Vector3& aDeltaLocation) noexcept;
		void AddLocalRotation(const Quaternion& aDeltaRotation) noexcept;
		void AddLocalRotationEulerDegrees(const Vector3& aDeltaEulerDegrees) noexcept;
		void AddLocalScale(const Vector3& aDeltaScale) noexcept;

		virtual void CopyFrom(const TransformComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override final;

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

		void OnBound() noexcept override;

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

	struct RootComponent{};
	struct SelectedInEditorComponent{};
	struct HiddenInGameComponent{};
	struct EntityDeleteRequestComponent{};

	struct FolderComponent
	{
		Folder Folder;
	};

	struct RLS_API ExposureSettings
	{
	public:
		NO_DISCARD float GetCompensation() const noexcept;
		NO_DISCARD float GetMinEV100() const noexcept;
		NO_DISCARD float GetMaxEV100() const noexcept;
		NO_DISCARD float GetSpeedUp() const noexcept;
		NO_DISCARD float GetSpeedDown() const noexcept;
		NO_DISCARD float GetLowPercent() const noexcept;
		NO_DISCARD float GetHighPercent() const noexcept;
		NO_DISCARD float GetHistogramMinEV100() const noexcept;
		NO_DISCARD float GetHistogramMaxEV100() const noexcept;

		void SetCompensation(float aCompensation) noexcept;
		void SetMinEV100(float aMinEV100) noexcept;
		void SetMaxEV100(float aMaxEV100) noexcept;
		void SetSpeedUp(float aSpeedUp) noexcept;
		void SetSpeedDown(float aSpeedDown) noexcept;
		void SetLowPercent(float aLowPercent) noexcept;
		void SetHighPercent(float aHighPercent) noexcept;
		void SetHistogramMinEV100(float aHistogramMinEV100) noexcept;
		void SetHistogramMaxEV100(float aHistogramMaxEV100) noexcept;
	private:
		float m_Compensation = 1.0f;
		float m_MinEV100 = -10.0f;
		float m_MaxEV100 = 20.0f;
		float m_SpeedUp = 3.0f;
		float m_SpeedDown = 1.0f;
		float m_LowPercent = 10.0f;
		float m_HighPercent = 90.0f;
		float m_HistogramMinEV100 = -10.0f;
		float m_HistogramMaxEV100 = 20.0f;
	};

	struct RLS_API PostProcessVolumeComponent
	{
		NO_DISCARD ExposureSettings& GetExposure() noexcept;
		NO_DISCARD const ExposureSettings& GetExposure() const noexcept;
		
		NO_DISCARD bool HasInfiniteExtent() const noexcept;
	private:
		ExposureSettings m_ExposureSettings;
		bool m_InfiniteExtent = true;
	};
}