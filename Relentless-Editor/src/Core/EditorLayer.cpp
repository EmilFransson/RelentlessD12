#include "EditorLayer.h"

namespace Relentless
{
	EditorLayer::EditorLayer(const std::string& layerName) noexcept
		: Layer{layerName}
	{
	}

	void EditorLayer::OnEvent(IEvent& event) noexcept
	{
		Editor::Get()->OnEvent(event);
	}

	void EditorLayer::OnImGuiRender() noexcept
	{
		Editor::Get()->OnImGuiRender();
	}

	void EditorLayer::OnAttach() noexcept
	{
		Editor::Get()->OnCreate();
	}

	void EditorLayer::OnDetach() noexcept
	{
		Editor::Get()->OnDestroy();
	}

	void EditorLayer::OnUpdate(const float deltaTime) noexcept
	{
		Editor::Get()->OnUpdate(deltaTime);
	}

	void EditorLayer::OnRender() noexcept
	{
		Editor::Get()->OnRender();
	}
}
