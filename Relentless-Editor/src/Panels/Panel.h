#pragma once
#include <Relentless.h>

namespace Relentless
{
	class IPanel
	{
	public:
		virtual ~IPanel() noexcept = default;

		virtual void Render() noexcept = 0;
		virtual void Update() noexcept {};
		[[nodiscard]] virtual bool OnEvent(IEvent&) noexcept;
	protected:
		[[nodiscard]] virtual bool OnKeyPressedEvent(KeyPressedEvent& event) noexcept { return false; };
		[[nodiscard]] virtual bool OnLeftMouseButtonPressedEvent(LeftMouseButtonPressedEvent& event) noexcept { return false; };
		[[nodiscard]] virtual bool OnLeftMouseButtonReleasedEvent(LeftMouseButtonReleasedEvent& event) noexcept { return false; };
		[[nodiscard]] virtual bool OnRightMouseButtonPressedEvent(RightMouseButtonPressedEvent& event) noexcept { return false; };
		[[nodiscard]] virtual bool OnRightMouseButtonReleasedEvent(RightMouseButtonReleasedEvent& event) noexcept { return false; };
		[[nodiscard]] virtual bool OnMiddleMouseButtonPressedEvent(MiddleMouseButtonPressedEvent& event) noexcept { return false; };
		[[nodiscard]] virtual bool OnMiddleMouseButtonReleasedEvent(MiddleMouseButtonReleasedEvent& event) noexcept { return false; };
		[[nodiscard]] virtual bool OnMouseWheelScrolledEvent(MouseWheelScrolledEvent& event) noexcept { return false; };
	};

	class PanelBase : public IPanel
	{
	public:
		PanelBase(const char* pName, ImGuiWindowFlags flags) noexcept;
		virtual ~PanelBase() noexcept override = default;

		[[nodiscard]] const Vector2u& GetContentRegionAvail() const noexcept;
		[[nodiscard]] const Vector2u& GetContentRegionMin() const noexcept;
		[[nodiscard]] const Vector2u& GetContentRegionMax() const noexcept;
		[[nodiscard]] FloatRect GetContentRegionInScreenSpace() const noexcept;
		[[nodiscard]] uint32 GetLastFrameFocused() const noexcept;
		[[nodiscard]] const Vector2u& GetPosition() const noexcept;
		[[nodiscard]] const Vector2u& GetSize() const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] bool IsDocked() const noexcept;
		[[nodiscard]] bool IsFocused() const noexcept;
		[[nodiscard]] bool IsHovered() const noexcept;
		[[nodiscard]] bool IsVisible() const noexcept;
	
		virtual void Render() noexcept override final;
		virtual void Update() noexcept override {};

		Broadcaster<void()> OnPostRender;
		Broadcaster<void(PanelBase*)> OnGainedFocus;
		Broadcaster<void(PanelBase*)> OnLostFocus;
		Broadcaster<void(PanelBase*, const Vector2u& newSize)> OnResized;
	protected:

		virtual void PreRender() noexcept {}
		virtual void OnRender() noexcept = 0;
		virtual void PostRender() noexcept {}

		void SetRoot(Ref<IWidget> pRoot) noexcept;
	private:
		std::string m_Name{};
		ImGuiWindowFlags m_Flags = ImGuiWindowFlags_None;

		Vector2u m_ContentRegionAvail	= Vector2u::Zero();
		Vector2u m_ContentRegionMin		= Vector2u::Zero();
		Vector2u m_ContentRegionMax		= Vector2u::Zero();
		Vector2u m_Position				= Vector2u::Zero();
		Vector2u m_Size					= Vector2u::Zero();

		uint32 m_LastFrameFocused = std::numeric_limits<uint32>::max();

		bool m_IsDocked		= false;
		bool m_IsFocused	= false;
		bool m_IsHovered	= false;
		bool m_IsVisible	= false;

		Ref<IWidget> m_pRoot = nullptr;
	};
}