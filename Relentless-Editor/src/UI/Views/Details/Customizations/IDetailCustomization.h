#pragma once
#include <Relentless.h>

namespace Relentless
{
	class IDetailLayoutBuilder;

	class IDetailCustomization
	{
	public:
		virtual ~IDetailCustomization() noexcept = default;
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept = 0;
	};
}