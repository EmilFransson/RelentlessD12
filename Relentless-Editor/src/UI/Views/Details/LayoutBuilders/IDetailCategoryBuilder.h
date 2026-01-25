#pragma once
#include "UI/Views/Details/DetailNode.h"

namespace Relentless
{
	class IDetailCategoryBuilder
	{
	public:
		explicit IDetailCategoryBuilder(const char* aName) noexcept;
		virtual ~IDetailCategoryBuilder() noexcept = default;

		NO_DISCARD DetailNode& AddProperty(const char* aPropertyName) noexcept;

		NO_DISCARD bool ExistsProperty(const char* aPropertyName) const noexcept;
		
		NO_DISCARD const std::vector<Ref<DetailNode>>& GetNodes() const noexcept;
		
		bool m_IsExpanded = true;
	private:
		std::vector<Ref<DetailNode>> m_Nodes;
		String m_Name;
	};
}