#pragma once

#include "Assets/AssetMeta.h"
#include "ECS/Component.h"

namespace Relentless
{
	class Environment;

	enum class ESkyLightCaptureMode : uint8 { Static = 0u, Realtime };
	enum class ESkyLightLowerHemisphereMode : uint8 { Environment = 0u, SolidColor };

	struct RLS_API SkyLightComponent : public ManagedComponent<SkyLightComponent>
	{
	public:
		struct DirtyRenderState{};

		inline static constexpr uint32 MIN_RADIANCE_MAP_SIZE = 8u;
		inline static constexpr uint32 MAX_RADIANCE_MAP_SIZE = 4096u;

		virtual void CopyFrom(const SkyLightComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override final;

		NO_DISCARD Ref<Environment> GetBlendEnvironment() const noexcept;
		NO_DISCARD const AssetHandle& GetBlendEnvironmentHandle() const noexcept;
		NO_DISCARD float GetBlendFactor() const noexcept;
		NO_DISCARD ESkyLightCaptureMode GetCaptureMode() const noexcept;
		NO_DISCARD Ref<Environment> GetPrimaryEnvironment() const noexcept;
		NO_DISCARD const AssetHandle& GetPrimaryEnvironmentHandle() const noexcept;
		NO_DISCARD const Color& GetLowerHemisphereColor() const noexcept;
		NO_DISCARD ESkyLightLowerHemisphereMode GetLowerHemisphereMode() const noexcept;
		NO_DISCARD float GetIntensity() const noexcept;
		NO_DISCARD uint32 GetRadianceMapSize() const noexcept;
		NO_DISCARD uint32 GetRealtimeMipsPerFrame() const noexcept;
		NO_DISCARD const Color& GetTintColor() const noexcept;

		NO_DISCARD bool HasAssignedBlendEnvironment() const noexcept;
		NO_DISCARD bool HasAssignedPrimaryEnvironment() const noexcept;

		void OnBound() noexcept override;

		void RemoveBlendEnvironment() noexcept;
		void RemovePrimaryEnvironment() noexcept;

		void SetBlendEnvironment(const AssetHandle& aHandle) noexcept;
		void SetBlendFactor(float aBlendFactor) noexcept;
		void SetCaptureMode(ESkyLightCaptureMode aCaptureMode) noexcept;
		void SetIntensity(float aIntensity) noexcept;
		void SetLowerHemisphereColor(const Color& aColor) noexcept;
		void SetLowerHemisphereMode(ESkyLightLowerHemisphereMode aMode) noexcept;
		void SetPrimaryEnvironment(const AssetHandle& aHandle) noexcept;
		void SetRadianceMapSize(uint32 aSize) noexcept;
		void SetRealtimeMipsPerFrame(uint32 aNumMips) noexcept;
		void SetTintColor(const Color& aTintColor) noexcept;
	private:
		AssetHandle m_PrimaryEnvironmentHandle = AssetHandle::INVALID;
		AssetHandle m_BlendEnvironmentHandle = AssetHandle::INVALID;
		Color m_LowerHemisphereColor = Colors::Black;
		Color m_TintColor = Colors::White;
		uint32 m_RadianceMapSize = 256u;
		uint32 m_RealtimeMipsPerFrame = 1u;
		float m_BlendFactor = 0.0f;
		float m_Intensity = 1.0f;
		ESkyLightCaptureMode m_CaptureMode = ESkyLightCaptureMode::Static;
		ESkyLightLowerHemisphereMode m_LowerHemisphereMode = ESkyLightLowerHemisphereMode::Environment;
	};
}