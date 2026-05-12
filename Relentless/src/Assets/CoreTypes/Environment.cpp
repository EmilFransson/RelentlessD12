#include "Environment.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/TextureCube.h"

#include "Utility/StringUtils.h"

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

	const AssetHandle& Environment::GetEnvironmentMapHandle() const noexcept
	{
		return m_EnvironmentMapHandle;
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

		if (m_EnvironmentMapHandle == aHandle)
			return;

		m_EnvironmentMapHandle = aHandle;
		NOTIFY_PROPERTY_CHANGED(m_EnvironmentMapHandle);
	}

	void Environment::SetIntensity(float aIntensity) noexcept
	{
		if (Math::AreValuesClose(m_Intensity, aIntensity))
			return;

		m_Intensity = aIntensity;
		NOTIFY_PROPERTY_CHANGED(m_Intensity);
	}

	void Environment::SetSolidColor(const Color& aColor) noexcept
	{
		if (m_SolidColor == aColor)
			return;

		m_SolidColor = aColor;
		NOTIFY_PROPERTY_CHANGED(m_SolidColor);
	}

	void Environment::SetSourceType(EEnvironmentSourceType aSourceType) noexcept
	{
		if (m_SourceType == aSourceType)
			return;

		m_SourceType = aSourceType;
		NOTIFY_PROPERTY_CHANGED(m_SourceType);
	}

	void Environment::PostLoad()
	{
		if (m_EnvironmentMapHandle.IsValid())
			AssetManager::LoadAsset(m_EnvironmentMapHandle);
	}

}