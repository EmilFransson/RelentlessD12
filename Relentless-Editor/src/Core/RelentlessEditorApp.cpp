#include "RelentlessEditorApp.h"
#include "EntryPoint.h"
#include "ImGui/ImGuiLayer.h"

#include "Subsystem/EditorViewportSubsystem.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 615; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = "D3D12\\"; }
namespace Relentless
{
	static LRESULT EditorWndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	}

	RelentlessEditor::RelentlessEditor(const ApplicationSpecification& applicationSpecification) noexcept
		: Application{applicationSpecification}
	{
	}

	RelentlessEditor::~RelentlessEditor() noexcept = default;

	void RelentlessEditor::Initialize() noexcept
	{
		m_pRenderer = MakeUnique<Renderer>(m_pGraphicsDevice);
		
		m_pImGuiLayer = MakeUnique<ImGuiLayer>(m_pGraphicsDevice);
		PushOverlay(m_pImGuiLayer.get());

		m_pEditorLayer = MakeUnique<EditorLayer>();
		PushLayer(m_pEditorLayer.get());

		Window::SetWndProcHook(&EditorWndProcHook);
	}

	//At this point all layers have finished both Updating & Rendering
	void RelentlessEditor::Update() noexcept
	{
		m_pRenderer->Render();
	}

	void RelentlessEditor::UIRenderBegin(CommandContext* aCommandContext) noexcept
	{
		m_pImGuiLayer->BeginFrame(m_pSwapchain->GetBackBuffer(), aCommandContext);
	}

	void RelentlessEditor::UIRenderEnd(CommandContext* aCommandContext) noexcept
	{
		m_pImGuiLayer->EndFrame(aCommandContext);
	}

	const UniquePtr<Renderer>& RelentlessEditor::GetRenderer() const noexcept
	{
		return m_pRenderer;
	}

	UniquePtr<Application> CreateApplication() noexcept
	{
		ApplicationSpecification applicationSpecification;
		applicationSpecification.Name = std::string("Relentless-Editor") + std::string(APPLICATION_SUFFIX);

		return UniquePtr<RelentlessEditor>(RLS_NEW RelentlessEditor(applicationSpecification));
	}
}