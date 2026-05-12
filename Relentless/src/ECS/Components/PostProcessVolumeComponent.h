#pragma once
#include "ECS/Component.h"

namespace Relentless
{
#define NOTIFY_NESTED_PROPERTY_CHANGED(parent_path, member) \
   ((void)(this->member), this->NotifyOwner(HashString(parent_path "." #member)))

	struct PostProcessVolumeComponent;

	template<typename TOwner>
	struct SubObject
	{
	public:
		explicit SubObject(TOwner* aOwner) noexcept
			: m_pOwner{ aOwner }{}

	protected:
		TOwner* m_pOwner = nullptr;

		void NotifyOwner(uint64 aHash) noexcept
		{
			static_assert(requires(TOwner * p, uint64 h) { p->NotifyPropertyChanged(h); }, "Owner must implement NotifyPropertyChanged(uint64)");
			RLS_ASSERT(m_pOwner, "SubObject owner not injected");
			
			m_pOwner->NotifyPropertyChanged(aHash);
		}

		friend TOwner;
	};

	enum class EAmbientOcclusionDepthPrecision : uint8 { F16 = 16u, F32 = 32u };
	enum class EAmbientOcclusionBlurRadius : uint8 { _2 = 2u, _4 = 4u };
	enum class EAmbientOcclusionStepcount : uint8 { _4 = 4u, _8 = 8u };

	struct RLS_API AmbientOcclusionSettings : public SubObject<PostProcessVolumeComponent>
	{
	public:
		explicit AmbientOcclusionSettings(PostProcessVolumeComponent* aOwner) noexcept;

		NO_DISCARD float GetBias() const noexcept;
		NO_DISCARD EAmbientOcclusionBlurRadius GetBlurRadius() const noexcept;
		NO_DISCARD float GetBlurSharpness() const noexcept;
		NO_DISCARD EAmbientOcclusionDepthPrecision GetDepthPrecision() const noexcept;
		NO_DISCARD float GetPowerExponent() const noexcept;
		NO_DISCARD float GetRadius() const noexcept;
		NO_DISCARD EAmbientOcclusionStepcount GetStepCount() const noexcept;

		NO_DISCARD bool IsBlurEnabled() const noexcept;
		NO_DISCARD bool IsEnabled() const noexcept;

		void SetBias(float aBias) noexcept;
		void SetBlurEnabled(bool aEnabled) noexcept;
		void SetBlurRadius(EAmbientOcclusionBlurRadius aBlurRadius) noexcept;
		void SetBlurSharpness(float aBlurSharpness) noexcept;
		void SetDepthPrecision(EAmbientOcclusionDepthPrecision aDepthPrecision) noexcept;
		void SetEnabled(bool aEnabled) noexcept;
		void SetPowerExponent(float aPowerExponent) noexcept;
		void SetRadius(float aRadius) noexcept;
		void SetStepCount(EAmbientOcclusionStepcount aStepCount) noexcept;
	private:
		float m_Radius = 2.f;
		float m_Bias = 0.1f;
		float m_PowerExponent = 2.f;
		float m_BlurSharpness = 16.f;
		EAmbientOcclusionBlurRadius m_BlurRadius = EAmbientOcclusionBlurRadius::_4;
		EAmbientOcclusionDepthPrecision m_DepthPrecision = EAmbientOcclusionDepthPrecision::F32;
		EAmbientOcclusionStepcount m_StepCount = EAmbientOcclusionStepcount::_8;
		bool m_BlurEnabled = true;
		bool m_IsEnabled = true;
	};

	struct RLS_API ExposureSettings : public SubObject<PostProcessVolumeComponent>
	{
	public:
		explicit ExposureSettings(PostProcessVolumeComponent* aOwner) noexcept;

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
		float m_SpeedUp = 6.0f;
		float m_SpeedDown = 5.0f;
		float m_LowPercent = 10.0f;
		float m_HighPercent = 90.0f;
		float m_HistogramMinEV100 = -10.0f;
		float m_HistogramMaxEV100 = 16.0f;
	};

	struct RLS_API PostProcessVolumeComponent : public ManagedComponent<PostProcessVolumeComponent>
	{
	public:
		friend struct SubObject<PostProcessVolumeComponent>;
		struct DirtyRenderState{};

		NO_DISCARD AmbientOcclusionSettings& GetAmbientOcclusion() noexcept;
		NO_DISCARD const AmbientOcclusionSettings& GetAmbientOcclusion() const noexcept;
		NO_DISCARD ExposureSettings& GetExposure() noexcept;
		NO_DISCARD const ExposureSettings& GetExposure() const noexcept;

		void CopyFrom(const PostProcessVolumeComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override;
		
		NO_DISCARD bool HasInfiniteExtent() const noexcept;

		void OnBound() noexcept override;
	private:
		void InjectSelf() noexcept;

		void NotifyPropertyChanged(uint64 aPropertyHash) noexcept;
	private:
		ExposureSettings m_ExposureSettings{ this };
		AmbientOcclusionSettings m_AmbientOcclusionSettings{ this };
		bool m_InfiniteExtent = true;
	};
}