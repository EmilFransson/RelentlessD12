#pragma once

#include "Assets/AssetMeta.h"
#include "ECS/Component.h"

namespace Relentless
{
	class Environment;
	class IAsset;

	struct RLS_API SkyBoxComponent : public ManagedComponent<SkyBoxComponent>
	{
	public:
		SkyBoxComponent() = default;
		SkyBoxComponent(SkyBoxComponent&&) noexcept = default;
		SkyBoxComponent& operator=(SkyBoxComponent&&) noexcept = default;
		virtual ~SkyBoxComponent() noexcept override;

		struct DirtyRenderState{};

		virtual void CopyFrom(const SkyBoxComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override final;

		NO_DISCARD Ref<Environment> GetBlendEnvironment() const noexcept;
		NO_DISCARD const AssetHandle& GetBlendEnvironmentHandle() const noexcept;
		NO_DISCARD float GetBlendFactor() const noexcept;
		NO_DISCARD float GetIntensity() const noexcept;
		NO_DISCARD float GetLODBias() const noexcept;
		NO_DISCARD Ref<Environment> GetPrimaryEnvironment() const noexcept;
		NO_DISCARD const AssetHandle& GetPrimaryEnvironmentHandle() const noexcept;
		NO_DISCARD const Color& GetTintColor() const noexcept;

		NO_DISCARD bool HasAssignedBlendEnvironment() const noexcept;
		NO_DISCARD bool HasAssignedPrimaryEnvironment() const noexcept;

		void OnBound() noexcept override final;

		void RemoveBlendEnvironment() noexcept;
		void RemovePrimaryEnvironment() noexcept;

		void SetBlendEnvironment(const AssetHandle& aHandle) noexcept;
		void SetBlendFactor(float aBlendFraction) noexcept;
		void SetIntensity(float aIntensity) noexcept;
		void SetLODBias(float aLODBias) noexcept;
		void SetPrimaryEnvironment(const AssetHandle& aHandle) noexcept;
		void SetTintColor(const Color& aColor) noexcept;
	private:
		void ConnectBlendEnvironment() noexcept;
		void ConnectPrimaryEnvironment() noexcept;

		void DetachBlendEnvironment() noexcept;
		void DetachPrimaryEnvironment() noexcept;

		void OnBlendEnvironmentAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept;
		void OnPrimaryEnvironmentAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept;
		void OnEnvironmentAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept;
	private:
		AssetHandle m_PrimaryEnvironmentHandle = AssetHandle::INVALID;
		AssetHandle m_BlendEnvironmentHandle = AssetHandle::INVALID;
		Color m_TintColor = Colors::White;
		float m_Intensity = 1.0f;
		float m_LodBias = 0.0f;
		float m_BlendFactor = 0.0f;
	};
}
