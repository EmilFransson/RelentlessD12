#pragma once
#include <Relentless.h>
#include "IWidget.h"

namespace Relentless
{
	class IPanel
	{
	public:
		virtual ~IPanel() noexcept;

		virtual void Render() noexcept = 0;
		virtual void Update() noexcept {};
		NO_DISCARD virtual bool OnEvent(IEvent&) noexcept;
	protected:
		NO_DISCARD virtual bool OnKeyPressedEvent(KeyPressedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnLeftMouseButtonPressedEvent(LeftMouseButtonPressedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnLeftMouseButtonReleasedEvent(LeftMouseButtonReleasedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnRightMouseButtonPressedEvent(RightMouseButtonPressedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnRightMouseButtonReleasedEvent(RightMouseButtonReleasedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnMiddleMouseButtonPressedEvent(MiddleMouseButtonPressedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnMiddleMouseButtonReleasedEvent(MiddleMouseButtonReleasedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnMouseWheelScrolledEvent(MouseWheelScrolledEvent&) noexcept { return false; };
	};

	class PanelBase : public IPanel
	{
	public:
		PanelBase(const char* pName, ImGuiWindowFlags flags) noexcept;
		virtual ~PanelBase() noexcept override;

		NO_DISCARD const Vector2u& GetContentRegionAvail() const noexcept;
		NO_DISCARD const Vector2u& GetContentRegionMin() const noexcept;
		NO_DISCARD const Vector2u& GetContentRegionMax() const noexcept;
		NO_DISCARD FloatRect GetContentRegionInScreenSpace() const noexcept;
		NO_DISCARD uint32 GetLastFrameFocused() const noexcept;
		NO_DISCARD const Vector2u& GetPosition() const noexcept;
		NO_DISCARD const Vector2u& GetSize() const noexcept;
		NO_DISCARD const String& GetName() const noexcept;
		NO_DISCARD bool IsDocked() const noexcept;
		NO_DISCARD bool IsFocused() const noexcept;
		NO_DISCARD bool IsHovered() const noexcept;
		NO_DISCARD bool IsVisible() const noexcept;
	
		virtual void Render() noexcept override final;

		void SetPadding(const Vector2& padding);

		virtual void Update() noexcept override {};

		Broadcaster<void()> OnPostRender;
		Broadcaster<void(PanelBase*)> OnGainedFocus;
		Broadcaster<void(PanelBase*)> OnLostFocus;
		Broadcaster<void(PanelBase*, const Vector2u& newSize)> OnResized;
	protected:

		virtual void PreRender() noexcept {}
		virtual void OnRender() noexcept {};
		virtual void PostRender() noexcept {}

		void SetRoot(Ref<IBaseWidget> pRoot) noexcept;
	private:
		String m_Name{};
		std::unordered_map<ImGuiStyleVar, ImVec2> m_Styles;

		Vector2u m_ContentRegionAvail	= Vector2u::Zero();
		Vector2u m_ContentRegionMin		= Vector2u::Zero();
		Vector2u m_ContentRegionMax		= Vector2u::Zero();
		Vector2u m_Position				= Vector2u::Zero();
		Vector2u m_Size					= Vector2u::Zero();

		ImGuiWindowFlags m_Flags = ImGuiWindowFlags_None;
		uint32 m_LastFrameFocused = std::numeric_limits<uint32>::max();

		bool m_IsDocked		= false;
		bool m_IsFocused	= false;
		bool m_IsHovered	= false;
		bool m_IsVisible	= false;

		Ref<IBaseWidget> m_pRoot = nullptr;
	};
}