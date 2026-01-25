#include "IDetailCategoryBuilder.h"

namespace Relentless
{
	IDetailCategoryBuilder::IDetailCategoryBuilder(const char* aName) noexcept
		: m_Name{ aName }
	{
	}

	DetailNode& IDetailCategoryBuilder::AddProperty(const char* aPropertyName) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddProperty]: Property already exists in category.");

		Ref<DetailNode> pNewNode = new DetailNode(aPropertyName);
		m_Nodes.push_back(pNewNode);
		return *m_Nodes.back();
	}

	bool IDetailCategoryBuilder::ExistsProperty(const char* aPropertyName) const noexcept
	{
		return std::ranges::any_of(m_Nodes, [aPropertyName](const Ref<DetailNode>& aNode) { return aNode->GetName() == aPropertyName; });
	}

	const std::vector<Ref<DetailNode>>& IDetailCategoryBuilder::GetNodes() const noexcept
	{
		return m_Nodes;
	}

}