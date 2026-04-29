#pragma once
#include "ECS/Component.h"

namespace Relentless
{
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

		Transform m_LocalTransform;
		mutable Matrix m_WorldMatrix = Matrix::Identity;
		uint32         m_LocalVersion = 0u;
		mutable uint32 m_WorldVersion = 0u;
		mutable uint32 m_LocalVersionSeenForWorld = 0u;
		mutable uint32 m_ParentWorldVersionSeen = 0u;

		Scene* m_Scene = nullptr;
	};
}