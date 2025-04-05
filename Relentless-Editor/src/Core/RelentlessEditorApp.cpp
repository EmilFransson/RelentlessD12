#include "RelentlessEditorApp.h"
#include "EntryPoint.h"
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 615; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = "D3D12\\"; }
namespace Relentless
{
	RelentlessEditor::RelentlessEditor(const ApplicationSpecification& applicationSpecification) noexcept
		: Application{applicationSpecification}
	{
	}

	void RelentlessEditor::Initialize() noexcept
	{
		m_pRenderer = std::make_unique<Renderer>(m_pGraphicsDevice);
		m_pEditorLayer = std::make_unique<EditorLayer>();
		PushLayer(m_pEditorLayer.get());
	}

	//At this point all layers have finished both Updating & Rendering
	void RelentlessEditor::Update() noexcept
	{
		GraphicsOptions options;
		options.HBAOPlusEnabled = true;
		options.SampleCount = 1u;

		std::vector<ViewportRenderView>& renderViews = m_pEditorLayer->GetEditor()->GetRenderViews();
		for (int i = 0; i < renderViews.size(); ++i)
		{
			renderViews[i].pTarget = m_pGraphicsDevice->CreateTexture(TextureDesc::Create2D(std::max(1.0f, renderViews[i].Viewport.GetWidth()), std::max(1.0f, renderViews[i].Viewport.GetHeight()), ResourceFormat::RGB10A2_UNORM, 1u, TextureFlag::ShaderResource), std::format("Target: {}", i).c_str());
			m_pRenderer->Render(m_pEditorLayer->GetEditor()->GetActiveScene(), &renderViews[i], options, renderViews[i].pTarget);
		}
	}

	const UniquePtr<Renderer>& RelentlessEditor::GetRenderer() const noexcept
	{
		return m_pRenderer;
	}

	const std::unique_ptr<Application> CreateApplication() noexcept
	{
		ApplicationSpecification applicationSpecification;
		applicationSpecification.Name = std::string("Relentless-Editor") + std::string(APPLICATION_SUFFIX);

		return std::unique_ptr<RelentlessEditor>(RLS_NEW RelentlessEditor(applicationSpecification));
	}
}