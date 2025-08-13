#pragma once

#include "Callback/Callback.h"
#include "IWidget.h"
#include "HorizontalBox.h"
#include "Label.h"
#include "VerticalBox.h"

namespace Relentless
{
	struct MenuEntry : public RefCounted<MenuEntry>
	{
		virtual ~MenuEntry() noexcept = default;
		virtual NO_DISCARD bool OnRender() noexcept = 0;

		ImVec2 RectMin = ImVec2(0.0f, 0.0f);
		ImVec2 RectMax = ImVec2(0.0f, 0.0f);
		bool Hovered = false;
		bool HighlightOnHover = true;

		bool IsHovered() const { return Hovered; }
	};

	struct MenuItem : public MenuEntry
	{
		virtual bool OnRender() noexcept override
		{
			ImGui::PushID((const void*)this);

			if (HighlightOnHover && Hovered && pWidget->IsEnabled())
			{
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				constexpr Color hoveredColor = Colors::RowFocusedSelectionColorDefault;
				pDrawList->AddRectFilled(RectMin, RectMax, ImGui::ColorConvertFloat4ToU32(ImVec4(hoveredColor.R(), hoveredColor.G(), hoveredColor.B(), hoveredColor.A())));
			}

			ImGui::BeginGroup();
			pWidget->Render();
			ImGui::EndGroup();

			RectMin = ImGui::GetItemRectMin();
			RectMax = ImGui::GetItemRectMax();
			Hovered = ImGui::IsItemHovered();
			
			const float fullWidth = RectMax.x - RectMin.x;
			const float height = RectMin.y - RectMax.y;
			bool triggered = false;

			if (OnClickedCallback.IsSet() && pWidget->IsEnabled())
			{
				if (ImGui::IsMouseHoveringRect(RectMin, RectMax) && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					OnClickedCallback();
					triggered = true;
				}
			}

			ImGui::PopID();

			return triggered;
		}

		Ref<IBaseWidget> pWidget = nullptr;
		Callback<void()> OnClickedCallback;
	};

	struct SubMenu : public MenuEntry
	{
		SubMenu(int level)
			: myPopupLevel{level}{ }

		virtual bool OnRender() noexcept override
		{
			char popup_id[64];
			sprintf(popup_id, "##submenu_%p", this);

			ImGui::PushID(this);

			bool anyChildIsOpen = false;
			for (auto& child : Entries)
			{
				if (SubMenu* pMenu = dynamic_cast<SubMenu*>(child.Get()))
					anyChildIsOpen |= pMenu->Open;
			}

			// Draw background
			if (isHoveredNow || anyChildIsOpen)
			{
				constexpr Color hoveredColor = Colors::RowFocusedSelectionColorDefault;
				ImGui::GetWindowDrawList()->AddRectFilled(RectMin, RectMax, ImGui::ColorConvertFloat4ToU32(ImVec4(hoveredColor.R(), hoveredColor.G(), hoveredColor.B(), hoveredColor.A())));
			}

			// Render the label
			ImGui::BeginGroup();
			pWidget->Render();
			ImGui::EndGroup();

			RectMin = ImGui::GetItemRectMin();
			RectMax = ImGui::GetItemRectMax();

			bool entryHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AllowWhenBlockedByPopup);
			bool submenuHovered = false;
			bool triggered = false;

			// Open popup if hovered
			if (entryHovered && !ImGui::IsPopupOpen(popup_id))
			{
				ImGui::OpenPopup(popup_id);
				Open = true;
				LastHoveredTime = ImGui::GetTime();
			}

			// Set popup position
			ImVec2 anchor = ImVec2(RectMin.x - ImGui::GetStyle().WindowPadding.x, RectMin.y - ImGui::GetStyle().WindowPadding.y);
			ImGui::SetNextWindowSize(ImVec2(Width, 0), ImGuiCond_Always);
			ImGui::SetNextWindowPos(anchor, ImGuiCond_Always, ImVec2(1.0f, 0.0f));

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			if (ImGui::BeginPopup(popup_id, ImGuiWindowFlags_NoMove))
			{
				submenuHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

				for (auto& child : Entries)
				{
					if (child->OnRender())
						triggered = true;
				}
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			isHoveredNow = entryHovered || submenuHovered;

			// --- Grace timer update ---
			if (isHoveredNow)
			{
				LastHoveredTime = ImGui::GetTime(); // Reset timer if still hovered
			}

			// Allow small grace period
			const double hoverGraceTime = 0.1;
			bool expired = (ImGui::GetTime() - LastHoveredTime) > hoverGraceTime;

			if (Open && !anyChildIsOpen && expired)
			{
				ImGuiContext& g = *ImGui::GetCurrentContext();
				if (myPopupLevel >= 0 && myPopupLevel < g.OpenPopupStack.Size)
					ImGui::ClosePopupToLevel(myPopupLevel, true);

				Open = false;
			}

			ImGui::PopID();
			return triggered;
		}

		Ref<IBaseWidget> pWidget = nullptr;
		std::vector<Ref<MenuEntry>> Entries;
		bool Open = false;
		int myPopupLevel = 0;
		double LastHoveredTime = 0.0f; // Track last time it was hovered
		bool isHoveredNow = false;
		float Width = 300.0f;
	};

	class ContextMenu : public IStylableWidget<ContextMenu>
	{
	public:
		void AddEntry(Ref<MenuEntry> pEntry) noexcept
		{
			m_Entries.push_back(pEntry);
		}

		float CalcDesiredWidth() const noexcept override { return 0.0f; }

		template<typename InstanceType>
		ContextMenu* OnClosed(InstanceType* instance, void(InstanceType::* method)()) noexcept
		{
			m_OnClosedCallback = [instance, method]() { return (instance->*method)(); };
			return this;
		}

		template<typename T>
		ContextMenu* OnClosed(T&& callback) noexcept
		{
			m_OnClosedCallback = Callback<void()>(std::forward<T>(callback));
			return this;
		}

		void OnRender() noexcept override
		{
			if (!m_IsOpen)
			{
				ImGui::OpenPopup("##ContextMenu");
				m_IsOpen = true;
			}

			constexpr Color bgColor = Colors::ContextMenuColorDefault;

			ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_Always);
			
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(bgColor.R(), bgColor.G(), bgColor.B(), bgColor.A()));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

			if (ImGui::BeginPopupContextWindow("##ContextMenu", ImGuiPopupFlags_None))
			{
				for (size_t i = 0u; i < m_Entries.size() && m_IsOpen; ++i)
				{
					if (m_Entries[i]->OnRender())
					{
						ImGui::ClosePopupToLevel(0, true);
						m_OnClosedCallback.ExecuteIfSet();
						m_IsOpen = false;
					}
				}

				ImGui::EndPopup();
			}
			else
			{
				m_OnClosedCallback.ExecuteIfSet();
				m_IsOpen = false;
			}

			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
		}

	private:
		std::vector<Ref<MenuEntry>> m_Entries;
		Callback<void()> m_OnClosedCallback;
		bool m_IsOpen = false;
	};

	class VerticalBox;

	class MenuBuilder : public RefCounted<MenuBuilder>
	{
	public:
		MenuBuilder() noexcept;
		virtual ~MenuBuilder() noexcept = default;

		template<typename InstanceType>
		MenuBuilder* AddMenuEntry(const String& label, const String& tooltip, const String& icon, InstanceType* instance, void(InstanceType::* method)(), bool enabled = true) noexcept
		{
			Ref<HorizontalBox> pBox = new HorizontalBox();
			pBox->SetSizePolicy(ESizePolicy::Stretch);

			if (!icon.empty())
			{
				Ref<Label> pLabel = new Label(icon, ImGui::GetIO().Fonts->Fonts[2]);
				pLabel->SetAlpha(0.5f);
				pBox->Add(pLabel);
			}

			Ref<Label> pLabel = new Label(label, ImGui::GetIO().Fonts->Fonts[2]);

			pBox->Add(pLabel);
			
			if (!tooltip.empty())
				pBox->SetTooltipText(tooltip);

			pBox->SetMargin(IntRect(40.0f, 0.0f, 0.0f, 0.0f));
			pBox->SetIsEnabled(enabled);

			Ref<MenuItem> pItem = new MenuItem();
			pItem->pWidget = std::move(pBox);
			pItem->OnClickedCallback = [instance, method]() { return (instance->*method)(); };

			if (!m_SubMenuStack.empty())
				m_SubMenuStack.top()->Entries.push_back(pItem);
			else
				m_pMenu->AddEntry(pItem);

			return this;
		}

		MenuBuilder* AddWidget(Ref<IBaseWidget> pWidget, const String& label, const String& tooltip) noexcept;

		NO_DISCARD Ref<ContextMenu> Build() noexcept;
		MenuBuilder* AddSection(const String& label) noexcept;
		MenuBuilder* BeginSubMenu(const String& label, const String& tooltip, const String& icon) noexcept;
		MenuBuilder* EndSubMenu() noexcept;
	private:
		Ref<ContextMenu> m_pMenu = nullptr;
		std::stack<Ref<SubMenu>> m_SubMenuStack;
	};
}