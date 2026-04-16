#include "Component.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Environment.h"

#include "Scene/Scene.h"

namespace Relentless
{
	void TransformComponent::AddWorldOffset(const Vector3& aDeltaLocation) noexcept
	{
		SetWorldLocation(GetWorldLocation() + aDeltaLocation);
	}

	void TransformComponent::AddWorldRotation(const Quaternion& aDeltaRotation) noexcept
	{
		SetWorldRotation(aDeltaRotation * GetWorldRotation());
	}

	void TransformComponent::AddWorldRotationEulerDegrees(const Vector3& aDeltaEulerDegrees) noexcept
	{
		SetWorldRotationEulerDegrees(GetWorldRotationEulerDegrees() + aDeltaEulerDegrees);
	}

	void TransformComponent::AddWorldScale(const Vector3& aDeltaScale) noexcept
	{
		SetWorldScale(GetWorldScale() + aDeltaScale);
	}

	void TransformComponent::AddLocalOffset(const Vector3& aDeltaLocation) noexcept
	{
		SetLocalLocation(GetLocalLocation() + aDeltaLocation);
	}

	void TransformComponent::AddLocalRotation(const Quaternion& aDeltaRotation) noexcept
	{
		SetLocalRotation(aDeltaRotation * GetLocalRotation());
	}

	void TransformComponent::AddLocalRotationEulerDegrees(const Vector3& aDeltaEulerDegrees) noexcept
	{
		SetLocalRotationEulerDegrees(GetLocalRotationEulerDegrees() + aDeltaEulerDegrees);
	}

	void TransformComponent::AddLocalScale(const Vector3& aDeltaScale) noexcept
	{
		SetLocalScale(GetLocalScale() + aDeltaScale);
	}
	
	void TransformComponent::CopyFrom(const TransformComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		LocalTransform = aOtherComponent.LocalTransform;
		WorldMatrix = aOtherComponent.WorldMatrix;
		LocalVersion = aOtherComponent.LocalVersion;
		WorldVersion = aOtherComponent.WorldVersion;
		LocalVersionSeenForWorld = aOtherComponent.LocalVersionSeenForWorld;
		ParentWorldVersionSeen = aOtherComponent.ParentWorldVersionSeen;

		m_Self = aThisEntity;
		m_EntityManager = &aEntityManager;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	Vector3 TransformComponent::GetWorldForward() const noexcept
	{
		return GetWorldMatrix().Forward();
	}
	
	const Matrix& TransformComponent::GetWorldMatrix() const noexcept
	{
		EnsureWorldUpToDate();
		return WorldMatrix;
	}
	
	Vector3 TransformComponent::GetWorldLocation() const noexcept
	{
		return GetWorldMatrix().Translation();
	}

	Quaternion TransformComponent::GetWorldRotation() const noexcept
	{
		Quaternion local = GetLocalRotation();
		local.Normalize();

		if (!Scene->HasParent(m_Self))
			return local;

		auto& parentTc = Scene->GetEntityManager().Get<TransformComponent>(Scene->GetParent(m_Self));
		Quaternion parentWorld = parentTc.GetWorldRotation();
		parentWorld.Normalize();

		Quaternion world = local * parentWorld;
		world.Normalize();
		return world;
	}

	Vector3 TransformComponent::GetWorldRotationEulerDegrees() const noexcept
	{
		return Math::RadToDeg(GetWorldRotation().ToEuler());
	}

	Vector3 TransformComponent::GetWorldRight() const noexcept
	{
		return GetWorldMatrix().Right();
	}
	
	Vector3 TransformComponent::GetWorldScale() const noexcept
	{
		Matrix world = GetWorldMatrix();

		Vector3    scale		= Vector3::One;
		Quaternion rotation		= Quaternion::Identity;
		Vector3    translation	= Vector3::Zero;

		RLS_DEBUG_ONLY(bool ok =) world.Decompose(scale, rotation, translation);
		RLS_ASSERT(ok, "[TransformComponent::GetWorldScale]: Failed to decompose world matrix.");

		return scale;
	}

	Vector3 TransformComponent::GetWorldUp() const noexcept
	{
		return GetWorldMatrix().Up();
	}

	Matrix TransformComponent::GetLocalMatrix() const noexcept
	{
		return BuildLocalMatrix(LocalTransform);
	}

	const Vector3& TransformComponent::GetLocalLocation() const noexcept
	{
		return LocalTransform.Location;
	}

	const Quaternion& TransformComponent::GetLocalRotation() const noexcept
	{
		return LocalTransform.Rotation;
	}
	Vector3 TransformComponent::GetLocalRotationEulerDegrees() const noexcept
	{
		return Math::RadToDeg(LocalTransform.Rotation.ToEuler());
	}

	const Vector3& TransformComponent::GetLocalScale() const noexcept
	{
		return LocalTransform.Scale;
	}

	void TransformComponent::OnBound() noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void TransformComponent::SetLocalLocation(const Vector3& aLocation) noexcept
	{
		LocalTransform.Location = aLocation;
		LocalVersion++;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void TransformComponent::SetLocalRotation(const Quaternion& aRotation) noexcept
	{
		LocalTransform.Rotation = aRotation;
		LocalVersion++;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void TransformComponent::SetLocalRotationEulerDegrees(const Vector3& aEulerDegrees) noexcept
	{
		const float yaw = Math::DegToRad(aEulerDegrees.y);
		const float pitch = Math::DegToRad(aEulerDegrees.x);
		const float roll = Math::DegToRad(aEulerDegrees.z);

		SetLocalRotation(Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll));
	}

	void TransformComponent::SetLocalScale(const Vector3& aScale) noexcept
	{
		LocalTransform.Scale = aScale;
		LocalVersion++;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void TransformComponent::SetWorldLocation(const Vector3& aLocation) noexcept
	{
		Matrix parentWorld = Matrix::Identity;

		if (Scene->HasParent(m_Self))
		{
			const entity parent = Scene->GetParent(m_Self);
			auto& parentTc = Scene->GetEntityManager().Get<TransformComponent>(parent);
			parentWorld = parentTc.GetWorldMatrix();
		}

		Matrix parentInv = Matrix::Identity;
		parentWorld.Invert(parentInv);

		const Vector3 localPosition = Vector3::Transform(aLocation, parentInv);

		LocalTransform.Location = localPosition;
		LocalVersion++;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void TransformComponent::SetWorldRotation(const Quaternion& aRotation) noexcept
	{
		Quaternion localRotation = aRotation;

		if (Scene->HasParent(m_Self))
		{
			auto& parentTc = Scene->GetEntityManager().Get<TransformComponent>(Scene->GetParent(m_Self));
			Quaternion parentRot = parentTc.GetWorldRotation();

			Quaternion parentInv = Quaternion::Identity;
			parentRot.Inverse(parentInv);
			localRotation = aRotation * parentInv;
		}

		LocalTransform.Rotation = localRotation;
		LocalTransform.Rotation.Normalize();
		LocalVersion++;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void TransformComponent::SetWorldRotationEulerDegrees(const Vector3& aEulerDegrees) noexcept
	{
		const float yaw = Math::DegToRad(aEulerDegrees.y);
		const float pitch = Math::DegToRad(aEulerDegrees.x);
		const float roll = Math::DegToRad(aEulerDegrees.z);

		SetWorldRotation(Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll));
	}

	void TransformComponent::SetWorldScale(const Vector3& aScale) noexcept
	{
		Vector3 localScale = aScale;

		if (Scene->HasParent(m_Self))
		{
			const entity parent = Scene->GetParent(m_Self);
			auto& parentTc = Scene->GetEntityManager().Get<TransformComponent>(parent);

			const Vector3 parentScale = parentTc.GetWorldScale();

			localScale.x /= Math::AreValuesClose(parentScale.x, 0.0f) ? 1.0f : parentScale.x;
			localScale.y /= Math::AreValuesClose(parentScale.y, 0.0f) ? 1.0f : parentScale.y;
			localScale.z /= Math::AreValuesClose(parentScale.z, 0.0f) ? 1.0f : parentScale.z;
		}

		LocalTransform.Scale = localScale;
		++LocalVersion;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void TransformComponent::EnsureWorldUpToDate() const noexcept
	{
		Matrix parentWorld = Matrix::Identity;
		uint32 parentWorldVersion = 0;

		const entity parent = Scene->HasParent(m_Self) ? Scene->GetParent(m_Self) : NULL_ENTITY;

		if (parent != NULL_ENTITY)
		{
			auto& parentTc = Scene->GetEntityManager().Get<TransformComponent>(parent);
			parentWorld = parentTc.GetWorldMatrix();
			parentWorldVersion = parentTc.WorldVersion;
		}

		const bool localChanged = (LocalVersion != LocalVersionSeenForWorld);
		const bool parentChanged = (ParentWorldVersionSeen != parentWorldVersion);

		if (localChanged || parentChanged)
		{
			WorldMatrix = ComposeWorld(parentWorld, LocalTransform);
			++WorldVersion;                      
			LocalVersionSeenForWorld = LocalVersion;
			ParentWorldVersionSeen = parentWorldVersion;
		}
	}

	inline Matrix TransformComponent::BuildLocalMatrix(const Transform& aLocalTransform) const noexcept
	{
		const Matrix S = Matrix::CreateScale(aLocalTransform.Scale);
		const Matrix R = Matrix::CreateFromQuaternion(aLocalTransform.Rotation);
		const Matrix T = Matrix::CreateTranslation(aLocalTransform.Location);
		return S * R * T;
	}

	inline Matrix TransformComponent::ComposeWorld(const Matrix& aParentWorldMatrix, const Transform& aLocalTransform) const noexcept
	{
		const Matrix localMatrix = BuildLocalMatrix(aLocalTransform);
		return localMatrix * aParentWorldMatrix;
	}

	const Color& LightBaseComponent::GetColor() const noexcept
	{
		return m_Color;
	}

	float LightBaseComponent::GetIntensity() const noexcept
	{
		return Intensity;
	}

	float LightBaseComponent::GetTemperature() const noexcept
	{
		return Temperature;
	}

	bool LightBaseComponent::IsUsingTemperature() const noexcept
	{
		return UseTemperature;
	}

	void LightBaseComponent::SetColor(const Color& aColor) noexcept
	{
		m_Color = aColor;
	}

	void LightBaseComponent::SetTemperature(float aTemperature) noexcept
	{
		Temperature = aTemperature;
	}

	void LightBaseComponent::SetUseTemperature(bool aUseTemperature) noexcept
	{
		UseTemperature = aUseTemperature;
	}

	float ExposureSettings::GetCompensation() const noexcept
	{
		return m_Compensation;
	}

	float ExposureSettings::GetMinEV100() const noexcept
	{
		return m_MinEV100;
	}

	float ExposureSettings::GetMaxEV100() const noexcept
	{
		return m_MaxEV100;
	}

	float ExposureSettings::GetSpeedUp() const noexcept
	{
		return m_SpeedUp;
	}

	float ExposureSettings::GetSpeedDown() const noexcept
	{
		return m_SpeedDown;
	}

	float ExposureSettings::GetLowPercent() const noexcept
	{
		return m_LowPercent;
	}

	float ExposureSettings::GetHighPercent() const noexcept
	{
		return m_HighPercent;
	}

	float ExposureSettings::GetHistogramMinEV100() const noexcept
	{
		return m_HistogramMinEV100;
	}

	float ExposureSettings::GetHistogramMaxEV100() const noexcept
	{
		return m_HistogramMaxEV100;
	}

	void ExposureSettings::SetCompensation(float aCompensation) noexcept
	{
		m_Compensation = aCompensation;
	}

	void ExposureSettings::SetMinEV100(float aMinEV100) noexcept
	{
		m_MinEV100 = aMinEV100;
	}

	void ExposureSettings::SetMaxEV100(float aMaxEV100) noexcept
	{
		m_MaxEV100 = aMaxEV100;
	}

	void ExposureSettings::SetSpeedUp(float aSpeedUp) noexcept
	{
		m_SpeedUp = aSpeedUp;
	}

	void ExposureSettings::SetSpeedDown(float aSpeedDown) noexcept
	{
		m_SpeedDown = aSpeedDown;
	}

	void ExposureSettings::SetLowPercent(float aLowPercent) noexcept
	{
		m_LowPercent = aLowPercent;
	}

	void ExposureSettings::SetHighPercent(float aHighPercent) noexcept
	{
		m_HighPercent = aHighPercent;
	}

	void ExposureSettings::SetHistogramMinEV100(float aHistogramMinEV100) noexcept
	{
		m_HistogramMinEV100 = aHistogramMinEV100;
	}

	void ExposureSettings::SetHistogramMaxEV100(float aHistogramMaxEV100) noexcept
	{
		m_HistogramMaxEV100 = aHistogramMaxEV100;
	}

	ExposureSettings& PostProcessVolumeComponent::GetExposure() noexcept
	{
		return m_ExposureSettings;
	}

	const ExposureSettings& PostProcessVolumeComponent::GetExposure() const noexcept
	{
		return m_ExposureSettings;
	}

	bool PostProcessVolumeComponent::HasInfiniteExtent() const noexcept
	{
		return m_InfiniteExtent;
	}

}
