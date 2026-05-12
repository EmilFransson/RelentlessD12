#include "MaterialDetailsView.h"

namespace Relentless
{
	MaterialDetailsView::MaterialDetailsView(const AssetHandle& aMaterialHandle) noexcept
	{
		SetHorizontalSizePolicy(ESizePolicy::Stretch);
		SetVerticalSizePolicy(ESizePolicy::Stretch);

		SetMaterial(aMaterialHandle);
	}

	MaterialDetailsView::~MaterialDetailsView() noexcept
	{
		TearDown();
	}

	void MaterialDetailsView::SetMaterial(const AssetHandle& aMaterialHandle) noexcept
	{
		m_DetailsContext.MaterialHandle = aMaterialHandle;
		m_DetailsContext.Material = AssetManager::Get<Material>(aMaterialHandle);

		SetContext(&m_DetailsContext);
		Rebuild<MaterialDetailsContext>();
		RequestRefresh();
	}
}