#pragma once
#include <Relentless.h>

#include "UI/Views/Details/Customizations/IDetailCustomization.h"

namespace Relentless
{
	class IDetailLayoutBuilder;

	class MeshRendererComponentDetailCustomization : public IDetailCustomization
	{
	public:
		virtual ~MeshRendererComponentDetailCustomization() noexcept override;
	protected:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;

		NO_DISCARD virtual bool ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept override;
	private:
		CallbackID m_OnMeshRendererComponentPropertyChangedCallbackID = INVALID_CALLBACK_ID;
	};
}