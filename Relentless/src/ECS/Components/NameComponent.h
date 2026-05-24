#pragma once
#include "ECS/Component.h"

namespace Relentless
{
	struct RLS_API NameComponent : public ComponentBase<NameComponent>
	{
	public:
		explicit NameComponent(const char* aName) noexcept;

		NO_DISCARD const String& GetName() const noexcept;

		void SetName(const String& aName) noexcept;
	private:
		String m_Name;
	};
}