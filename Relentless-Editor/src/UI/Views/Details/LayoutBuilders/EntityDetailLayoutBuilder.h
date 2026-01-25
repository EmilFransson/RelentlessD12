#pragma once
#include <Relentless.h>

#include "IDetailLayoutBuilder.h"
#include "UI/Views/Details/Customizations/IDetailCustomization.h"

namespace Relentless
{
	class EntityDetailLayoutBuilder : public IDetailLayoutBuilder
	{
	public:
		EntityDetailLayoutBuilder(IDetailsView* aDetailsView, Scene& aScene) noexcept;
		virtual ~EntityDetailLayoutBuilder() noexcept override = default;

		NO_DISCARD std::vector<UniquePtr<IDetailCustomization>>& GetCustomizations() noexcept;
		NO_DISCARD Scene& GetScene() const noexcept;

		NO_DISCARD std::vector<Ref<DetailNode>> Rebuild() noexcept;
		void RequestRefresh() noexcept;
	private:
		std::vector<UniquePtr<IDetailCustomization>> m_Customizations;
		Scene& m_Scene;
	};
}