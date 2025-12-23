#include "IDetailLayoutBuilder.h"
#include "../IDetailsView.h"

namespace Relentless
{
	IDetailLayoutBuilder::IDetailLayoutBuilder(IDetailsView* pDetailView) noexcept
		: m_pView{ pDetailView }	
	{
	}

	IDetailCategoryBuilder& IDetailLayoutBuilder::EditCategory(const char* aName) noexcept
	{
		auto [it, inserted] = m_Categories.try_emplace(aName, std::make_unique<IDetailCategoryBuilder>(aName));
		return *(it->second);
	}

	IDetailsView* IDetailLayoutBuilder::GetDetailsView() const noexcept
	{
		return m_pView;
	}

}