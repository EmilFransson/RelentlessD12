#pragma once
#include "UI/Views/Details/Context/EntityDetailsContext.h"
#include "UI/Views/Details/Customizations/IDetailCustomization.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"

namespace Relentless
{
	template<typename ComponentType>
	class EntityDetailCustomization : public IDetailCustomization
	{
	public:
		NO_DISCARD IDetailLayoutBuilder* GetDetailLayoutBuilder() const noexcept;
		NO_DISCARD IDetailsView* GetDetailsView() const noexcept;

		NO_DISCARD virtual bool IsEntityInspected(entity aEntity) const noexcept;
	protected:
		virtual ~EntityDetailCustomization() noexcept override = default;

		virtual void CustomizeDetails(const SharedPtr<IDetailLayoutBuilder>& aDetailLayoutBuilder) noexcept override final;

		NO_DISCARD virtual bool ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept override;
	private:
		WeakPtr<IDetailLayoutBuilder> m_pWeakDetailLayoutBuilder;
	};

	template<typename ComponentType>
	IDetailLayoutBuilder* EntityDetailCustomization<ComponentType>::GetDetailLayoutBuilder() const noexcept
	{
		if (SharedPtr<IDetailLayoutBuilder> pLayoutBuilder = m_pWeakDetailLayoutBuilder.lock())
			return pLayoutBuilder.get();

		return nullptr;
	}

	template<typename ComponentType>
	IDetailsView* EntityDetailCustomization<ComponentType>::GetDetailsView() const noexcept
	{
		if (SharedPtr<IDetailLayoutBuilder> pLayoutBuilder = m_pWeakDetailLayoutBuilder.lock())
			return pLayoutBuilder->GetDetailsView();

		return nullptr;
	}

	template<typename ComponentType>
	void EntityDetailCustomization<ComponentType>::CustomizeDetails(const SharedPtr<IDetailLayoutBuilder>& aDetailLayoutBuilder) noexcept
	{
		m_pWeakDetailLayoutBuilder = aDetailLayoutBuilder;
	}

	template<typename ComponentType>
	bool EntityDetailCustomization<ComponentType>::IsEntityInspected(entity aEntity) const noexcept
	{
		if (SharedPtr<IDetailLayoutBuilder> pLayoutBuilder = m_pWeakDetailLayoutBuilder.lock())
		{
			const EntityDetailsContext& context = pLayoutBuilder->GetDetailsView()->GetContext<EntityDetailsContext>();
			if (context.Entities.empty())
				return false;
			
			return std::ranges::any_of(context.Entities, [&context, aEntity](entity aInspectedEntity) { return aEntity == aInspectedEntity && context.EntityManager->Has<ComponentType>(aEntity); });
		}

		return false;
	}

	template<typename ComponentType>
	bool EntityDetailCustomization<ComponentType>::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		const EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		if (context.Entities.empty())
			return false;

		return std::ranges::all_of(context.Entities, [&context](entity aEntity) { return context.EntityManager->Has<ComponentType>(aEntity); });
	}
}