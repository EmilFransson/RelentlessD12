#include "EditorLayer.h"

namespace Relentless
{
	EditorLayer::EditorLayer(const std::string& layerName) noexcept
		: Layer{layerName}
	{
	}

	void EditorLayer::OnEvent(IEvent& event) noexcept
	{
		if (UIManager::Get().OnEvent(event))
			return;

		m_pEditor->OnEvent(event);
	}

	void EditorLayer::OnImGuiRender() noexcept
	{
		UIManager::Get().OnRender();
		m_pEditor->OnImGuiRender();
	}

	void EditorLayer::OnAttach() noexcept
	{
		m_pEditor = std::make_shared<Editor>();
		m_pEditor->OnCreate();
	}

	void EditorLayer::OnDetach() noexcept
	{
		m_pEditor->OnDestroy();
	}

	void EditorLayer::OnUpdate(const float deltaTime) noexcept
	{
		UIManager::Get().OnUpdate();
		m_pEditor->OnUpdate(deltaTime);
	}

	void EditorLayer::OnRender() noexcept
	{
		m_pEditor->OnRender();
	}
	
	const std::shared_ptr<Editor>& EditorLayer::GetEditor() const noexcept
	{
		return m_pEditor;
	}

}
