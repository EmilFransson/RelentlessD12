#pragma once
#include <Relentless.h>

#include "IDetailCustomization.h"

#include "UI/Views/Details/LayoutBuilders/EntityDetailLayoutBuilder.h"
#include "UI/Widgets/Button.h"

namespace Relentless
{
	class EntityDetailLayoutBuilder;

	class EntityDetailCustomization : public IDetailCustomization
	{
	public:
		virtual ~EntityDetailCustomization() noexcept override = default;

		template<typename Func, typename DefaultValueT>
		NO_DISCARD Ref<Button> AddRevertButtonWidget(Func&& aCallback, const DefaultValueT& aRevertValue, bool aIsVisible) noexcept;

		NO_DISCARD const std::vector<entity>& GetInspectedEntities() const noexcept;
		NO_DISCARD entity GetPrimaryEntity() const noexcept;

		template<typename TComponent, typename Func>
		void EditComponentData(Scene& aScene, entity aEntity, Func&& aFunction) noexcept
		{
			aScene.MarkDirty();

			aFunction(aScene.GetEntityManager().Get<TComponent>(aEntity));
		}

		template<typename TComponent, typename Func>
		void EditComponentData(Scene& aScene, Func&& aFunction) noexcept
		{
			aScene.MarkDirty();

			std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity)
				{
					aFunction(aEntity, aScene.GetEntityManager().Get<TComponent>(aEntity));
				});
		}

		void OnRevertButtonHoverBegin(Button* aButton) noexcept;
		void OnRevertButtonHoverEnd(Button* aButton) noexcept;

	protected:
		EntityDetailLayoutBuilder* m_pBuilder = nullptr;
	};

	template<typename Func, typename DefaultValueT>
	Ref<Button> EntityDetailCustomization::AddRevertButtonWidget(Func&& aCallback, const DefaultValueT& aRevertValue, bool aIsVisible) noexcept
	{
		Ref<Button> pRevertButton = new Button(ICON_FA_ARROW_ROTATE_LEFT, Vector2(32.0f, 32.0f));
		pRevertButton->OnClicked(
			[revertButton = pRevertButton.Get(),
			callback = std::forward<Func>(aCallback),
			revertValue = aRevertValue]() mutable
			{
				callback(revertValue);
				revertButton->SetIsVisible(false);
			});
		pRevertButton->SetBackgroundColor(Colors::Transparent);
		pRevertButton->SetActiveColor(Colors::Transparent);
		pRevertButton->SetHoverColor(Colors::Transparent);
		pRevertButton->SetBorderColor(Colors::Transparent);
		pRevertButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
		pRevertButton->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
		pRevertButton->OnMouseEnter(this, &EntityDetailCustomization::OnRevertButtonHoverBegin);
		pRevertButton->OnMouseExit(this, &EntityDetailCustomization::OnRevertButtonHoverEnd);
		pRevertButton->SetIsVisible(aIsVisible);

		return pRevertButton;
	}
}