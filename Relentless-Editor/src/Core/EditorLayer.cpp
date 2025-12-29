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

		Editor::Get()->OnEvent(event);
	}

	void EditorLayer::OnImGuiRender() noexcept
	{
		UIManager::Get().OnRender();
		Editor::Get()->OnImGuiRender();
	}

	void EditorLayer::OnAttach() noexcept
	{
		//m_pEditor = std::make_shared<Editor>();
		Editor::Get()->OnCreate();
	}

	void EditorLayer::OnDetach() noexcept
	{
		Editor::Get()->OnDestroy();
	}

	void EditorLayer::OnUpdate(const float deltaTime) noexcept
	{
		UIManager::Get().OnUpdate();
		Editor::Get()->OnUpdate(deltaTime);
	}

	void EditorLayer::OnRender() noexcept
	{
		Editor::Get()->OnRender();
	}
}
