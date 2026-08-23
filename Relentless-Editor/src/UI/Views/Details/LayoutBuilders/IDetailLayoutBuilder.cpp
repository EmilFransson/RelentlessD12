#include "IDetailLayoutBuilder.h"
#include "../IDetailsView.h"

namespace Relentless
{
	IDetailLayoutBuilder::IDetailLayoutBuilder(IDetailsView* pDetailView) noexcept
		: m_pView{ pDetailView }	
	{
	}

	IDetailLayoutBuilder::~IDetailLayoutBuilder() noexcept
	{
		TearDown();
	}

	void IDetailLayoutBuilder::CollapseAll() noexcept
	{
		m_pView->GetTreeView()->CollapseAll();
	}

	IDetailCategoryBuilder& IDetailLayoutBuilder::EditCategory(const char* aName) noexcept
	{
		auto [it, inserted] = m_Categories.try_emplace(aName, MakeUnique<IDetailCategoryBuilder>(aName, *this));
		return *(it->second);
	}

	void IDetailLayoutBuilder::ExpandAll() noexcept
	{
		m_pView->GetTreeView()->ExpandAll();
	}

	void IDetailLayoutBuilder::ForceRefreshDetails() noexcept
	{
		m_pView->RequestRefresh();
	}

	IDetailsView* IDetailLayoutBuilder::GetDetailsView() const noexcept
	{
		return m_pView;
	}

	void IDetailLayoutBuilder::TearDown() noexcept
	{
		for (const auto& customization : m_Customizations)
		{
			if (customization->ShouldCustomize(*this))
				customization->OnDestroy(*this);
		}
		
		m_Categories.clear();
		m_Customizations.clear();
	}

}