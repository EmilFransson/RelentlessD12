#pragma once
#include "IDetailCategoryBuilder.h" 

namespace Relentless
{
	class IDetailsView;

	class IDetailLayoutBuilder
	{
	public:
		IDetailLayoutBuilder(IDetailsView* pDetailView) noexcept;
		virtual ~IDetailLayoutBuilder() noexcept = default;

		NO_DISCARD IDetailCategoryBuilder& EditCategory(const char* aName) noexcept;
	protected:
		std::unordered_map<String, UniquePtr<IDetailCategoryBuilder>> m_Categories;
		IDetailsView* m_pView = nullptr;
	};
}