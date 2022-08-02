#include "ImguiLayer.h"
#include "../Graphics/D3D12Core.h"
#include "../Window.h"
namespace Relentless
{
	std::unique_ptr<DescriptorHeap> ImguiLayer::m_pDescriptorHeap{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> ImguiLayer::m_pUITexture{ nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE ImguiLayer::my_texture_srv_cpu_handle{ };
	D3D12_GPU_DESCRIPTOR_HANDLE ImguiLayer::my_texture_srv_gpu_handle{ };

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

		static bool dockspaceOpen = true;

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
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
		static bool showWindow = true;
		ImGui::ShowDemoWindow(&showWindow);

		//if (ImGui::BeginMainMenuBar())
		//{
		//	if (ImGui::BeginMenu("Options"))
		//	{
		//		ImGui::MenuItem("Save...");
		//		ImGui::EndMenu();
		//	}
		//
		//	ImGui::EndMainMenuBar();
		//}

		//static bool show = true;
		//ImGui::Begin("Hello", &show);
		//ImGui::End();
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
		
		/*ImGui does not account for gamma correction out-of-the-box.
		  The following code snippet, as supplied by Chilitomatonoodle (youtube username),
		  converts the colors of ImGui so that it plays nice with DX11 gamma correction.*/
		const auto ToLinear = [](float x) {
			if (x <= 0.04045f)
			{
				return x / 12.92f;
			}
			else
			{
				return std::pow((x + 0.055f) / 1.055f, 2.4f);
			}};
		for (auto& c : std::span{ ImGui::GetStyle().Colors, size_t(ImGuiCol_COUNT) })
		{
			c = { ToLinear(c.x), ToLinear(c.y), ToLinear(c.z), c.w };
		}

		m_pDescriptorHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2u, true);
		
		//Descriptor heap handles:
		UINT handle_increment = D3D12Core::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		int descriptor_index = 1; // The descriptor table index to use (not normally a hard-coded constant, but in this case we'll assume we have slot 1 reserved for us)
		my_texture_srv_cpu_handle = m_pDescriptorHeap->GetCPUStartHandle();
		my_texture_srv_cpu_handle.ptr += (handle_increment * descriptor_index);
		my_texture_srv_gpu_handle = m_pDescriptorHeap->GetGPUStartHandle();
		my_texture_srv_gpu_handle.ptr += (handle_increment * descriptor_index);

		//UI TEXTURE
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC resourceDescriptor{};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDescriptor.Alignment = 0u;
		resourceDescriptor.Width = 800;
		resourceDescriptor.Height = 600;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		resourceDescriptor.SampleDesc = { 1u, 0u };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		clearValue.Color[0] = DirectX::Colors::Brown.f[0];
		clearValue.Color[1] = DirectX::Colors::Brown.f[1];
		clearValue.Color[2] = DirectX::Colors::Brown.f[2];
		clearValue.Color[3] = DirectX::Colors::Brown.f[3];

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDescriptor,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(&m_pUITexture)
		));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = resourceDescriptor.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pUITexture.Get(), &srvDesc, my_texture_srv_cpu_handle));
		
		NAME_D12_OBJECT(m_pUITexture, L"UI TEXTURE");

		ImGui_ImplWin32_Init(::GetActiveWindow());
		ImGui_ImplDX12_Init
		(
			D3D12Core::GetDevice().Get(),
			D3D12Core::GetNrOfBufferedFrames(),
			DXGI_FORMAT_R8G8B8A8_UNORM,
			m_pDescriptorHeap->GetDescriptorHeapInterface().Get(),
			m_pDescriptorHeap->GetCPUStartHandle(),
			m_pDescriptorHeap->GetGPUStartHandle()
		);
	}

	void ImguiLayer::OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept
	{
		//Descriptor heap handles:
		UINT handle_increment = D3D12Core::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		int descriptor_index = 1; // The descriptor table index to use (not normally a hard-coded constant, but in this case we'll assume we have slot 1 reserved for us)
		my_texture_srv_cpu_handle = m_pDescriptorHeap->GetCPUStartHandle();
		my_texture_srv_cpu_handle.ptr += (handle_increment * descriptor_index);
		my_texture_srv_gpu_handle = m_pDescriptorHeap->GetGPUStartHandle();
		my_texture_srv_gpu_handle.ptr += (handle_increment * descriptor_index);

		//UI TEXTURE
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC resourceDescriptor{};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDescriptor.Alignment = 0u;
		resourceDescriptor.Width = width;
		resourceDescriptor.Height = height;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		resourceDescriptor.SampleDesc = { 1u, 0u };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		clearValue.Color[0] = DirectX::Colors::Brown.f[0];
		clearValue.Color[1] = DirectX::Colors::Brown.f[1];
		clearValue.Color[2] = DirectX::Colors::Brown.f[2];
		clearValue.Color[3] = DirectX::Colors::Brown.f[3];

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDescriptor,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(&m_pUITexture)
		));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = resourceDescriptor.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pUITexture.Get(), &srvDesc, my_texture_srv_cpu_handle));

		NAME_D12_OBJECT(m_pUITexture, L"UI TEXTURE");
	}
}