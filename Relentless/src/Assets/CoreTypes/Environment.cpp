#include "Environment.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/TextureCube.h"

namespace Relentless
{
	Environment::Environment(const UUID& aUUID) noexcept
		:AssetBase<Environment>(aUUID)
	{
	}

	Environment::~Environment() noexcept = default;

	Ref<TextureCube> Environment::GetEnvironmentMap() const noexcept
	{
		RLS_ASSERT(m_EnvironmentMapHandle != AssetHandle::INVALID, "[Environment::GetEnvironmentMap]: Environment map asset handle is invalid.");
		return AssetManager::Get<TextureCube>(m_EnvironmentMapHandle);
	}

	float Environment::GetIntensity() const noexcept
	{
		return m_Intensity;
	}

	const Color& Environment::GetSolidColor() const noexcept
	{
		return m_SolidColor;
	}

	EEnvironmentSourceType Environment::GetSourceType() const noexcept
	{
		return m_SourceType;
	}

	bool Environment::HasValidEnvironmentMap() const noexcept
	{
		return m_EnvironmentMapHandle.IsValid();
	}

	bool Environment::SerializeCore(IArchive& aArchive) noexcept
	{
		return
			aArchive.Process(m_EnvironmentMapHandle) &&
			aArchive.Process(m_SolidColor) &&
			aArchive.Process(m_Intensity) &&
			aArchive.Process(m_SourceType) &&
			aArchive.IsValid();
	}

	void Environment::SetEnvironmentMapHandle(const AssetHandle& aHandle) noexcept
	{
		RLS_ASSERT(aHandle != AssetHandle::INVALID, "[Environment::SetEnvironmentMapHandle]: Asset handle is invalid.");
		RLS_ASSERT(aHandle.Type == TextureCube::StaticType(), "[Environment::SetEnvironmentMapHandle]: Asset handle type is invalid.");

		m_EnvironmentMapHandle = aHandle;
	}

	void Environment::SetIntensity(float aIntensity) noexcept
	{
		m_Intensity = aIntensity;
	}

	void Environment::SetSolidColor(const Color& aColor) noexcept
	{
		m_SolidColor = aColor;
	}

	void Environment::SetSourceType(EEnvironmentSourceType aSourceType) noexcept
	{
		m_SourceType = aSourceType;
	}
}