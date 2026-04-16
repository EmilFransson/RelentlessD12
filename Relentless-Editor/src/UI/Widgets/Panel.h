#pragma once
#include <Relentless.h>
#include "Event/EditorEvents.h"

#include "ImGui/ImGuiIncludes.h"

namespace Relentless
{
	class IBaseWidget;

	class IPanel
	{
	public:
		virtual ~IPanel() noexcept;

		virtual void Render() noexcept = 0;
		virtual void Update() noexcept {};
		virtual bool OnEvent(IEvent&) noexcept;
	protected:
		NO_DISCARD virtual bool OnKeyPressedEvent(KeyPressedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnKeyReleasedEvent(KeyReleasedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnLeftMouseButtonPressedEvent(LeftMouseButtonPressedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnLeftMouseButtonReleasedEvent(LeftMouseButtonReleasedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnRightMouseButtonPressedEvent(RightMouseButtonPressedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnRightMouseButtonReleasedEvent(RightMouseButtonReleasedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnMiddleMouseButtonPressedEvent(MiddleMouseButtonPressedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnMiddleMouseButtonReleasedEvent(MiddleMouseButtonReleasedEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnMouseWheelScrolledEvent(MouseWheelScrolledEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnMouseBeginDragEvent(MouseBeginDragEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnMouseDragEvent(MouseDragEvent&) noexcept { return false; };
		NO_DISCARD virtual bool OnMouseEndDragEvent(MouseEndDragEvent&) noexcept { return false; };
	};

	class PanelBase : public IPanel
	{
	public:
		PanelBase(const char* aName, ImGuiWindowFlags someFlags = ImGuiWindowFlags_None) noexcept;
		virtual ~PanelBase() noexcept override;

		NO_DISCARD virtual bool AcceptsMouseInput() const noexcept;

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
		NO_DISCARD virtual bool IsViewportPanel() const noexcept { return false; };
	
		virtual void Render() noexcept override final;

		void SetPadding(const Vector2& aPadding);

		virtual void Update() noexcept override {};

		Broadcaster<void(PanelBase*)> OnActivated;
		Broadcaster<void(PanelBase*)> OnClose;

		Broadcaster<void(PanelBase*)> OnCollapsed;
		Broadcaster<void(PanelBase*)> OnRestored;

		Broadcaster<void(PanelBase*)> OnDocked;
		Broadcaster<void(PanelBase*)> OnUndocked;

		Broadcaster<void()> OnPostRender;
		Broadcaster<void(PanelBase*)> OnGainedFocus;
		Broadcaster<void(PanelBase*)> OnLostFocus;
		Broadcaster<void(PanelBase*, const Vector2u& newSize)> OnResized;
		Broadcaster<void(PanelBase*)> OnMoved;
		Broadcaster<void(PanelBase*)> OnMouseEnter;
		Broadcaster<void(PanelBase*)> OnMouseExit;
	protected:
		virtual void PreRender() noexcept {}
		virtual void OnRender() noexcept {}
		virtual void PostRender() noexcept {}

		void SetRoot(Ref<IBaseWidget> aRoot) noexcept;
	private:
		void SyncActivation() noexcept;
		void SyncDocking() noexcept;
		void SyncFocus() noexcept;
		void SyncHover() noexcept;
		void SyncPosition() noexcept;
		void SyncSize() noexcept;
		void SyncContentRegion() noexcept;
		void SyncVisibility() noexcept;
	private:
		String m_Name{};
		std::unordered_map<ImGuiStyleVar, ImVec2> m_Styles;

		Vector2u m_ContentRegionAvail	= Vector2u::Zero();
		Vector2u m_ContentRegionMin		= Vector2u::Zero();
		Vector2u m_ContentRegionMax		= Vector2u::Zero();
		Vector2u m_Position				= Vector2u::Zero();
		Vector2u m_Size					= Vector2u::Zero();

		ImGuiWindowFlags m_Flags	= ImGuiWindowFlags_None;
		uint32 m_LastFrameFocused	= std::numeric_limits<uint32>::max();

		bool m_IsDocked		= false;
		bool m_IsFocused	= false;
		bool m_IsHovered	= false;
		bool m_IsVisible	= false;

		Ref<IBaseWidget> m_pRoot = nullptr;
	};
}