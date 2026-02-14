#pragma once
#include <Relentless.h>

#include "Details/DetailCustomizationRegistry.h"

namespace Relentless
{
	class DetailsModule : public IModule
	{
	public:
		NO_DISCARD const DetailCustomizationRegistry& GetRegistry() const noexcept;
	protected:
		void OnLoad() override;
	private:
		DetailCustomizationRegistry m_DetailCustomizationRegistry;
	};
}