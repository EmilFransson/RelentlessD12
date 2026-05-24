#include "NameComponent.h"

namespace Relentless
{
	NameComponent::NameComponent(const char* aName) noexcept
		: m_Name{ aName }
	{
	}

	const String& NameComponent::GetName() const noexcept
	{
		return m_Name;
	}

	void NameComponent::SetName(const String& aName) noexcept
	{
		if (aName == m_Name)
			return;

		m_Name = aName;
		NOTIFY_PROPERTY_CHANGED(m_Name);
	}
}