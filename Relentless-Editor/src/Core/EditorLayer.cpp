#include "EditorLayer.h"

namespace Relentless
{
	EditorLayer::EditorLayer(const std::string& layerName) noexcept
		: Layer{layerName}
	{
	}

	void EditorLayer::OnEvent(IEvent& event) noexcept
	{
		m_pEditor->OnEvent(event);
	}

	void EditorLayer::OnImGuiRender() noexcept
	{
		m_pEditor->OnImGuiRender();
	}

	void EditorLayer::OnAttach() noexcept
	{
		m_pEditor = std::make_unique<Editor>();
		m_pEditor->OnCreate();
	}

	void EditorLayer::OnDetach() noexcept
	{
		m_pEditor->OnDestroy();
	}

	void EditorLayer::OnUpdate(const float deltaTime) noexcept
	{
		m_pEditor->OnUpdate(deltaTime);
	}

	void EditorLayer::OnRender() noexcept
	{
		m_pEditor->OnRender();
	}
	
	void EditorLayer::OnPostRender() noexcept
	{
		m_pEditor->OnPostRender();
	}

	const UniquePtr<Editor>& EditorLayer::GetEditor() const noexcept
	{
		return m_pEditor;
	}

}
