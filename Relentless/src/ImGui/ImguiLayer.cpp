#include "ImguiLayer.h"
#include "../Graphics/D3D12Core.h"
#include "../Core/Window.h"
#include "../Graphics/MemoryManager.h"
namespace Relentless
{
	std::unique_ptr<DescriptorHeap> ImguiLayer::m_pDescriptorHeap{ nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE ImguiLayer::my_texture_srv_cpu_handle{ };
	D3D12_GPU_DESCRIPTOR_HANDLE ImguiLayer::my_texture_srv_gpu_handle{ };
	std::shared_ptr<RenderTexture> ImguiLayer::m_pUITexture{ nullptr };

	ImguiLayer::ImguiLayer() noexcept
		:Layer("ImGuiLayer")
	{
	}

	ImguiLayer::~ImguiLayer() noexcept
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void ImguiLayer::BeginFrame() noexcept
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		static bool dockspaceOpen = true;

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();
	}

	void ImguiLayer::EndFrame() noexcept
	{
		ImGui::Render();
		DXCall_STD(D3D12Core::GetCommandList()->SetDescriptorHeaps(1, m_pDescriptorHeap->GetDescriptorHeapInterface().GetAddressOf()));
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), D3D12Core::GetCommandList().Get());
		
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImguiLayer::OnImGuiRender() noexcept
	{
		//static bool showWindow = true;
		//ImGui::ShowDemoWindow(&showWindow);
	}

	void ImguiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable;
		ImGui::StyleColorsDark();
		
		std::string openSansFontPath = std::string(ENGINE_ASSET_DIRECTORY) + std::string("/Fonts/opensans/");
		io.Fonts->AddFontFromFileTTF((std::string(openSansFontPath) + std::string("OpenSans-Bold.ttf")).c_str(), 18.0f);
		io.Fonts->AddFontFromFileTTF((std::string(openSansFontPath) + std::string("OpenSans-Bold.ttf")).c_str(), 26.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF((std::string(openSansFontPath) + std::string("OpenSans-Regular.ttf")).c_str(), 18.0f);

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };
		
		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		
		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		
		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		
		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		
		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		m_pDescriptorHeap = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2u, true));
		
		//Descriptor heap handles:
		UINT handle_increment = D3D12Core::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		int descriptor_index = 1; // The descriptor table index to use (not normally a hard-coded constant, but in this case we'll assume we have slot 1 reserved for us)
		my_texture_srv_cpu_handle = m_pDescriptorHeap->GetCPUStartHandle();
		my_texture_srv_cpu_handle.ptr += (handle_increment * descriptor_index);
		my_texture_srv_gpu_handle = m_pDescriptorHeap->GetGPUStartHandle();
		my_texture_srv_gpu_handle.ptr += (handle_increment * descriptor_index);

		//UI TEXTURE
		RenderTextureSpecification textureSpecification = {};
		textureSpecification.Width = 800u;
		textureSpecification.Height = 600u;
		textureSpecification.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureSpecification.MultiSampleCount = 1u;
		textureSpecification.CreateSRV = false;
		textureSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::Brown.f);

		m_pUITexture = std::move(RenderTexture::Create(textureSpecification, "Main UI RenderTexture"));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = m_pUITexture->GetFormat();
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1u;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pUITexture->GetInterface().Get(), &srvDesc, my_texture_srv_cpu_handle));

		ImGui_ImplWin32_Init(::GetActiveWindow());
		ImGui_ImplDX12_Init
		(
			D3D12Core::GetDevice().Get(),
			D3D12Core::GetNrOfBufferedFrames(),
			DXGI_FORMAT_R10G10B10A2_UNORM,
			m_pDescriptorHeap->GetDescriptorHeapInterface().Get(),
			m_pDescriptorHeap->GetCPUStartHandle(),
			m_pDescriptorHeap->GetGPUStartHandle()
		);
	}

	void ImguiLayer::OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept
	{
		RenderTextureSpecification textureSpecification = {};
		textureSpecification.Width = width;
		textureSpecification.Height = height;
		textureSpecification.Format = m_pUITexture->GetFormat();
		textureSpecification.MultiSampleCount = 1u;
		textureSpecification.CreateSRV = false;
		textureSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::Brown);
		
		//MOVE TO Renderer3D:

		MemoryManager::Get().DestroyResource(std::move(m_pUITexture));

		m_pUITexture = std::move(RenderTexture::Create(textureSpecification, "Main UI Texture"));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = m_pUITexture->GetFormat();
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1u;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pUITexture->GetInterface().Get(), &srvDesc, my_texture_srv_cpu_handle));
	}
}