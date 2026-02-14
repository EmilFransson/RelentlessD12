#pragma once
#include <Relentless.h>

namespace Relentless
{
	class IPropertyHandleBase : public RefCounted<IPropertyHandleBase>
	{
	public:
		IPropertyHandleBase() noexcept = default;
		virtual ~IPropertyHandleBase() noexcept = default;
	};
}