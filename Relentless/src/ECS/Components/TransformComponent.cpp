#include "TransformComponent.h"

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
		m_LocalTransform = aOtherComponent.m_LocalTransform;
		m_WorldMatrix = aOtherComponent.m_WorldMatrix;
		m_LocalVersion = aOtherComponent.m_LocalVersion;
		m_WorldVersion = aOtherComponent.m_WorldVersion;
		m_LocalVersionSeenForWorld = aOtherComponent.m_LocalVersionSeenForWorld;
		m_ParentWorldVersionSeen = aOtherComponent.m_ParentWorldVersionSeen;

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
		return m_WorldMatrix;
	}

	Vector3 TransformComponent::GetWorldLocation() const noexcept
	{
		return GetWorldMatrix().Translation();
	}

	Quaternion TransformComponent::GetWorldRotation() const noexcept
	{
		Quaternion local = GetLocalRotation();
		local.Normalize();

		if (!m_Scene->HasParent(m_Self))
			return local;

		auto& parentTc = m_Scene->GetEntityManager().Get<TransformComponent>(m_Scene->GetParent(m_Self));
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

		Vector3    scale = Vector3::One;
		Quaternion rotation = Quaternion::Identity;
		Vector3    translation = Vector3::Zero;

		RLS_DEBUG_ONLY(bool ok = ) world.Decompose(scale, rotation, translation);
		RLS_ASSERT(ok, "[TransformComponent::GetWorldScale]: Failed to decompose world matrix.");

		return scale;
	}

	Vector3 TransformComponent::GetWorldUp() const noexcept
	{
		return GetWorldMatrix().Up();
	}

	Matrix TransformComponent::GetLocalMatrix() const noexcept
	{
		return BuildLocalMatrix(m_LocalTransform);
	}

	const Vector3& TransformComponent::GetLocalLocation() const noexcept
	{
		return m_LocalTransform.Location;
	}

	const Quaternion& TransformComponent::GetLocalRotation() const noexcept
	{
		return m_LocalTransform.Rotation;
	}
	Vector3 TransformComponent::GetLocalRotationEulerDegrees() const noexcept
	{
		return Math::RadToDeg(m_LocalTransform.Rotation.ToEuler());
	}

	const Vector3& TransformComponent::GetLocalScale() const noexcept
	{
		return m_LocalTransform.Scale;
	}

	void TransformComponent::OnBound() noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void TransformComponent::SetLocalLocation(const Vector3& aLocation) noexcept
	{
		m_LocalTransform.Location = aLocation;
		m_LocalVersion++;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LocalTransform.Location);
	}

	void TransformComponent::SetLocalRotation(const Quaternion& aRotation) noexcept
	{
		m_LocalTransform.Rotation = aRotation;
		m_LocalVersion++;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LocalTransform.Rotation);
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
		m_LocalTransform.Scale = aScale;
		m_LocalVersion++;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LocalTransform.Scale);
	}

	void TransformComponent::SetWorldLocation(const Vector3& aLocation) noexcept
	{
		Matrix parentWorld = Matrix::Identity;

		if (m_Scene->HasParent(m_Self))
		{
			const entity parent = m_Scene->GetParent(m_Self);
			auto& parentTc = m_Scene->GetEntityManager().Get<TransformComponent>(parent);
			parentWorld = parentTc.GetWorldMatrix();
		}

		Matrix parentInv = Matrix::Identity;
		parentWorld.Invert(parentInv);

		const Vector3 localPosition = Vector3::Transform(aLocation, parentInv);

		m_LocalTransform.Location = localPosition;
		m_LocalVersion++;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LocalTransform.Location);
	}

	void TransformComponent::SetWorldRotation(const Quaternion& aRotation) noexcept
	{
		Quaternion localRotation = aRotation;

		if (m_Scene->HasParent(m_Self))
		{
			auto& parentTc = m_Scene->GetEntityManager().Get<TransformComponent>(m_Scene->GetParent(m_Self));
			Quaternion parentRot = parentTc.GetWorldRotation();

			Quaternion parentInv = Quaternion::Identity;
			parentRot.Inverse(parentInv);
			localRotation = aRotation * parentInv;
		}

		m_LocalTransform.Rotation = localRotation;
		m_LocalTransform.Rotation.Normalize();
		m_LocalVersion++;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LocalTransform.Rotation);
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

		if (m_Scene->HasParent(m_Self))
		{
			const entity parent = m_Scene->GetParent(m_Self);
			auto& parentTc = m_Scene->GetEntityManager().Get<TransformComponent>(parent);

			const Vector3 parentScale = parentTc.GetWorldScale();

			localScale.x /= Math::AreValuesClose(parentScale.x, 0.0f) ? 1.0f : parentScale.x;
			localScale.y /= Math::AreValuesClose(parentScale.y, 0.0f) ? 1.0f : parentScale.y;
			localScale.z /= Math::AreValuesClose(parentScale.z, 0.0f) ? 1.0f : parentScale.z;
		}

		m_LocalTransform.Scale = localScale;
		++m_LocalVersion;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LocalTransform.Scale);
	}

	void TransformComponent::EnsureWorldUpToDate() const noexcept
	{
		Matrix parentWorld = Matrix::Identity;
		uint32 parentWorldVersion = 0;

		const entity parent = m_Scene->HasParent(m_Self) ? m_Scene->GetParent(m_Self) : NULL_ENTITY;

		if (parent != NULL_ENTITY)
		{
			auto& parentTc = m_Scene->GetEntityManager().Get<TransformComponent>(parent);
			parentWorld = parentTc.GetWorldMatrix();
			parentWorldVersion = parentTc.m_WorldVersion;
		}

		const bool localChanged = (m_LocalVersion != m_LocalVersionSeenForWorld);
		const bool parentChanged = (m_ParentWorldVersionSeen != parentWorldVersion);

		if (localChanged || parentChanged)
		{
			m_WorldMatrix = ComposeWorld(parentWorld, m_LocalTransform);
			++m_WorldVersion;
			m_LocalVersionSeenForWorld = m_LocalVersion;
			m_ParentWorldVersionSeen = parentWorldVersion;
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
}