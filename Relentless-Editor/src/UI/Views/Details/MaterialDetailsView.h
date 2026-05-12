#pragma once
#include "IDetailsView.h"

#include "UI/Views/Details/Context/MaterialDetailsContext.h"
namespace Relentless
{
	class MaterialDetailsView : public IDetailsView
	{
	public:
		explicit MaterialDetailsView(const AssetHandle& aMaterialHandle) noexcept;
		virtual ~MaterialDetailsView() noexcept;

		void SetMaterial(const AssetHandle& aMaterialHandle) noexcept;
	private:
		MaterialDetailsContext m_DetailsContext;
	};
}