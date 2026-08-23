#pragma once
#include "EntityDetailCustomization.h"

namespace Relentless
{
	class PostProcessVolumeComponentDetailCustomization : public EntityDetailCustomization<PostProcessVolumeComponent>
	{
		using IDetailCustomization::CustomizeDetails;
	protected:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	private:
		void CustomizeAmbientOcclusionDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext, IDetailsView* aDetailsView) noexcept;
		void CustomizeBloomDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext) noexcept;
		void CustomizeExposureDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext) noexcept;
	};
}