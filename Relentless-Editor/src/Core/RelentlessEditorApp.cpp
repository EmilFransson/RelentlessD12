#include "RelentlessEditorApp.h"
#include "EntryPoint.h"
#include "ImGui/ImGuiLayer.h"

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

		WindowEx::SetWndProcHook(&EditorWndProcHook);
	}

	//At this point all layers have finished both Updating & Rendering
	void RelentlessEditor::Update() noexcept
	{
		GraphicsOptions options;
		options.HBAOPlusEnabled = true;
		options.SampleCount = 1u;

		std::vector<ViewportRenderView>& renderViews = Editor::Get()->GetRenderViews();
		for (size_t i = 0; i < renderViews.size(); ++i)
		{
			const uint32 width	= static_cast<uint32>(Math::Max(1.0f, Math::Min(renderViews[i].Viewport.GetWidth(), (float)WindowEx::GetDisplaySize().x)));
			const uint32 height = static_cast<uint32>(Math::Max(1.0f, Math::Min(renderViews[i].Viewport.GetHeight(), (float)WindowEx::GetDisplaySize().y)));

			if (!m_pColorTarget || m_pColorTarget->GetWidth() != width || m_pColorTarget->GetHeight() != height)
				m_pColorTarget = m_pGraphicsDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::RGB10A2_UNORM, 1u, TextureFlag::ShaderResource), std::format("Target: {}", i).c_str());

			renderViews[i].pTarget = m_pColorTarget;
			m_pRenderer->Render(Editor::Get()->GetActiveScene(), &renderViews[i], options, renderViews[i].pTarget);
		}
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