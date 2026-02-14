#include "IAsset.h"

namespace Relentless
{
	IAsset::IAsset(const UUID& aUUID) noexcept
		: m_UUID(aUUID)
	{
	}

	const String& IAsset::GetName() const noexcept
	{
		return m_Name;
	}

	const UUID& IAsset::GetUUID() const noexcept
	{
		return m_UUID;
	}

	void IAsset::SetName(const String& aName) noexcept
	{
		m_Name = aName;
	}
}