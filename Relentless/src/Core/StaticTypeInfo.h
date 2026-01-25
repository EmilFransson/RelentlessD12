#pragma once
#include <StaticTypeInfo/type_index.h>

namespace Relentless
{
	using namespace static_type_info;
	using TypeIndex = static_type_info::TypeIndex;

	struct INVALID_TYPE
	{
		inline static constexpr TypeIndex StaticType() noexcept
		{
			static constexpr TypeIndex typeIndex = getTypeIndex<INVALID_TYPE>();
			return typeIndex;
		}
	};
}