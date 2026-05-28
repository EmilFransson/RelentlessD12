#include "EntityComponentDefinition.h"

namespace Relentless
{
	IEntityComponentDefinition::IEntityComponentDefinition(const EntityComponentDescriptor& aDescriptor) noexcept
		: m_Descriptor(aDescriptor)
	{}

	bool IEntityComponentDefinition::CanShowInEditor() const noexcept
	{
		return EnumHasAnyFlags(m_Descriptor.Flags, EEntityComponentFlags::ShowInEditor);
	}

	const String& IEntityComponentDefinition::GetCategory() const noexcept
	{
		return m_Descriptor.Category;
	}

	const String& IEntityComponentDefinition::GetDescription() const noexcept
	{
		return m_Descriptor.Description;
	}

	const String& IEntityComponentDefinition::GetDisplayName() const noexcept
	{
		return m_Descriptor.DisplayName;
	}

	const String& IEntityComponentDefinition::GetIcon() const noexcept
	{
		return m_Descriptor.Icon;
	}

}
