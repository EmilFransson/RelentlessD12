#include "EditorGrid.h"

namespace Relentless
{
	constexpr int	EDITOR_GRID_VERTEX_COUNT = 2;
	constexpr int	EDITOR_GRID_INSTANCE_COUNT = 800;
	constexpr int	EDITOR_GRID_INSTANCE_COUNT_HALF_SIZE = EDITOR_GRID_INSTANCE_COUNT / 2;
	constexpr int   EDITOR_GRID_MAJOR_EVERY = 10;
	constexpr float EDITOR_GRID_LINE_LENGTH = 10'000.0f;
	constexpr float EDITOR_GRID_FAMILY_CENTER = 200.0f;

	EditorGrid::EditorGrid(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pDevice{ aGraphicsDevice }
	{
		BuildInstanceBuffer();
	}

	void EditorGrid::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		if (m_Dirty)
			BuildInstanceBuffer();

		RenderPassInfo info;
		info.RenderTargets[0].pTarget = aSceneTextures.pLDRColorTarget;
		info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargetCount++;

		info.DepthStencilTarget.pTarget = aSceneTextures.pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::Preserve;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::Preserve;

		aCommandContext.InsertResourceBarrier(aSceneTextures.pLDRColorTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pDepthTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		aCommandContext.BeginRenderPass(info);

		aCommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		aCommandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());

		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Alpha);
		psoDesc.SetAlphaToCoverageEnable(false);
		psoDesc.SetName("Editor Grid");
		psoDesc.SetVertexShader("EditorGridShader", "vs_main");
		psoDesc.SetPixelShader("EditorGridShader", "ps_main");
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthWrite(false);
		psoDesc.SetLineAntiAliasingEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_GREATER_EQUAL);
		psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
		psoDesc.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGB10A2_UNORM, ResourceFormat::D32_FLOAT, 1);

		aCommandContext.SetPipelineState(m_pDevice->GetOrCreatePipeline(psoDesc));

		Renderer::BindViewData(aCommandContext, aRenderView);

		struct
		{
			Matrix BatchDataTransformVerticalMatrix = Matrix::Identity;
			Matrix BatchDataTransformHoriontalMatrix = Matrix::Identity;
			uint32 InstanceDataIndex = 0xFFFFFFFF;
			float DistanceFade = 0.0f;
			float HeightFade = 0.0f;
			float MaxOpacity = 1.0f;
		} params;

		const float spacing = std::max(m_Spacing, 0.001f);

		const DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYawFromVector(
			{ m_RotationRadians.x, m_RotationRadians.y, m_RotationRadians.z });
		const DirectX::XMMATRIX invRot = DirectX::XMMatrixTranspose(rot);

		const float snap = spacing * static_cast<float>(EDITOR_GRID_MAJOR_EVERY);

		const DirectX::XMVECTOR camWorld = DirectX::XMVectorSet(aRenderView.Location.x, aRenderView.Location.y, aRenderView.Location.z, 1.0f);

		DirectX::XMFLOAT3 camLocal;
		DirectX::XMStoreFloat3(&camLocal, DirectX::XMVector3TransformCoord(camWorld, invRot));

		camLocal.x = std::floor(camLocal.x / snap) * snap;
		camLocal.y = 0.0f;
		camLocal.z = std::floor(camLocal.z / snap) * snap;

		DirectX::XMFLOAT3 snapped;
		DirectX::XMStoreFloat3(&snapped, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&camLocal), rot));

		const DirectX::XMMATRIX place = DirectX::XMMatrixTranslation(snapped.x, snapped.y, snapped.z);

		// Vertical — instances 0..399, local z in [-400, -1]
		{
			DirectX::XMMATRIX world =
				DirectX::XMMatrixScaling(EDITOR_GRID_LINE_LENGTH, 1.0f, spacing)
				* DirectX::XMMatrixTranslation(0.0f, 0.0f, EDITOR_GRID_FAMILY_CENTER * spacing)
				* rot
				* place;
			DirectX::XMStoreFloat4x4(&params.BatchDataTransformVerticalMatrix, world);
		}

		// Horizontal — instances 400..799, local z in [0, 399]
		{
			DirectX::XMMATRIX world =
				DirectX::XMMatrixScaling(EDITOR_GRID_LINE_LENGTH, 1.0f, spacing)
				* DirectX::XMMatrixTranslation(0.0f, 0.0f, -EDITOR_GRID_FAMILY_CENTER * spacing)
				* DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2)
				* rot
				* place;
			DirectX::XMStoreFloat4x4(&params.BatchDataTransformHoriontalMatrix, world);
		}

		params.InstanceDataIndex = m_pInstancesStructuredBuffer->GetSRVIndex();

		params.DistanceFade = m_DistanceFade * spacing;
		params.HeightFade = m_HeightFade;
		params.MaxOpacity = m_MaxOpacity;

		aCommandContext.BindRootCBV(BindingSlot::PerPass, (const void*)&params, sizeof(params));
		aCommandContext.Draw(0, EDITOR_GRID_VERTEX_COUNT, 0, EDITOR_GRID_INSTANCE_COUNT);

		aCommandContext.EndRenderPass();
		
		aCommandContext.InsertResourceBarrier(aSceneTextures.pLDRColorTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	float EditorGrid::GetDistanceFade() const noexcept
	{
		return m_DistanceFade;
	}

	float EditorGrid::GetHeightFade() const noexcept
	{
		return m_HeightFade;
	}

	float EditorGrid::GetHeightOffset() const noexcept
	{
		return m_HeightOffset;
	}

	const Color& EditorGrid::GetMajorColor() const noexcept
	{
		return m_ColorMajor;
	}

	const Color& EditorGrid::GetMinorColor() const noexcept
	{
		return m_ColorMinor;
	}

	float EditorGrid::GetMaxOpacity() const noexcept
	{
		return m_MaxOpacity;
	}

	const Vector3& EditorGrid::GetRotationRadians() const noexcept
	{
		return m_RotationRadians;
	}

	float EditorGrid::GetSpacing() const noexcept
	{
		return m_Spacing;
	}

	void EditorGrid::SetDistanceFade(float aFade) noexcept
	{
		m_DistanceFade = aFade;
	}

	void EditorGrid::SetHeightFade(float aFade) noexcept
	{
		m_HeightFade = aFade;
	}

	void EditorGrid::SetHeightOffset(float aHeightOffset) noexcept
	{
		if (Math::AreValuesClose(m_HeightOffset, aHeightOffset))
			return;

		m_HeightOffset = aHeightOffset;
		m_Dirty = true;
	}

	void EditorGrid::SetMajorColor(const Color& aColor) noexcept
	{
		if (m_ColorMajor == aColor)
			return;

		m_ColorMajor = aColor;
		m_Dirty = true;
	}

	void EditorGrid::SetMinorColor(const Color& aColor) noexcept
	{
		if (m_ColorMinor == aColor)
			return;

		m_ColorMinor = aColor;
		m_Dirty = true;
	}

	void EditorGrid::SetMaxOpacity(float aMaxOpacity) noexcept
	{
		m_MaxOpacity = aMaxOpacity;
	}

	void EditorGrid::SetRotationRadians(const Vector3& aRotationRadians) noexcept
	{
		m_RotationRadians = aRotationRadians;
	}

	void EditorGrid::SetSpacing(float aSpacing) noexcept
	{
		m_Spacing = Math::Max(aSpacing, 0.01f);
	}

	void EditorGrid::BuildInstanceBuffer()
	{
		struct InstanceData
		{
			DirectX::XMFLOAT3 Position;
			float padding1;
			struct
			{
				float R;
				float G;
				float B;
			} Color;
			float padding;
		};

		std::vector<InstanceData> instances;
		instances.reserve(EDITOR_GRID_INSTANCE_COUNT);

		for (int i = -EDITOR_GRID_INSTANCE_COUNT_HALF_SIZE; i < EDITOR_GRID_INSTANCE_COUNT_HALF_SIZE; ++i)
		{
			InstanceData& instanceData = instances.emplace_back();
			instanceData.Position = DirectX::XMFLOAT3(0.0f, m_HeightOffset, static_cast<float>(i));
			const bool isMajor = (i % EDITOR_GRID_MAJOR_EVERY) == 0;
			const Color& c = isMajor ? m_ColorMajor : m_ColorMinor;
			instanceData.Color.R = Colors::SRGBToLinear(c.R());
			instanceData.Color.G = Colors::SRGBToLinear(c.G());
			instanceData.Color.B = Colors::SRGBToLinear(c.B());
		}

		m_pInstancesStructuredBuffer = m_pDevice->CreateBuffer(BufferDesc::CreateStructured(static_cast<uint32>(instances.size()), sizeof(InstanceData)), "Editor Grid Instances", instances.data());
		m_Dirty = false;
	}
}