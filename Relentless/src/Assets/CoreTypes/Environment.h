#pragma once
#include "Assets/IAsset.h"
#include "Assets/AssetMeta.h"

#include "Core/DLLExport.h"

namespace Relentless
{
	class TextureCube;
	
	enum class EEnvironmentSourceType : uint8 { Cubemap = 0u, SolidColor };

	class RLS_API Environment : public AssetBase<Environment>
	{
	public:
		Environment(const UUID& aUUID) noexcept;
		virtual ~Environment() noexcept override;

		static constexpr const UUID& PersistentType()
		{
			static constexpr UUID uid = UUID{ 0x33dfce5d, 0xba0d, 0x4766, { 0x91, 0x72, 0x5f, 0x79, 0x98, 0xe9, 0x10, 0x87 } };
			return uid;
		}

		NO_DISCARD Ref<TextureCube> GetEnvironmentMap() const noexcept;
		NO_DISCARD const AssetHandle& GetEnvironmentMapHandle() const noexcept;
		NO_DISCARD float GetIntensity() const noexcept;
		NO_DISCARD const Color& GetSolidColor() const noexcept;
		NO_DISCARD EEnvironmentSourceType GetSourceType() const noexcept;

		NO_DISCARD bool HasValidEnvironmentMap() const noexcept;

		virtual bool SerializeCore(IArchive& aArchive) noexcept override final;
		void SetEnvironmentMapHandle(const AssetHandle& aHandle) noexcept;
		void SetIntensity(float aIntensity) noexcept;
		void SetSolidColor(const Color& aColor) noexcept;
		void SetSourceType(EEnvironmentSourceType aSourceType) noexcept;
	private:
		virtual void PostLoad() override;
	private:
		AssetHandle m_EnvironmentMapHandle = AssetHandle::INVALID;
		Color m_SolidColor = Colors::Black;
		float m_Intensity = 1.0f;
		EEnvironmentSourceType m_SourceType = EEnvironmentSourceType::SolidColor;
	};
}