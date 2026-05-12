#pragma once
#include "UI/Views/Details/Customizations/IDetailCustomization.h"

namespace Relentless
{
	struct EntityDetailsContext;
	class IDetailCategoryBuilder;
	class IDetailLayoutBuilder;
	class IDetailsView;

	class PostProcessVolumeComponentDetailCustomization : public IDetailCustomization
	{
	protected:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;

		NO_DISCARD virtual bool ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept override;
	private:
		void CustomizeAmbientOcclusionDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext, IDetailsView* aDetailsView) noexcept;
		void CustomizeExposureDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext) noexcept;
	};
}