#include "EditorGrid.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"
#include "Graphics/RHI/Texture.h"

namespace Relentless
{
	constexpr int EDITOR_GRID_VERTEX_COUNT = 2;
	constexpr int EDITOR_GRID_INSTANCE_COUNT = 800;
	constexpr int EDITOR_GRID_INSTANCE_COUNT_HALF_SIZE = EDITOR_GRID_INSTANCE_COUNT / 2;

	EditorGrid::EditorGrid(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{

		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Alpha);
		psoDesc.SetAlphaToCoverageEnable(false);
		psoDesc.SetName("Editor Grid");
		psoDesc.SetVertexShader("EditorGridShader", "vs_main");
		psoDesc.SetPixelShader("EditorGridShader", "ps_main");
		psoDesc.SetDepthWrite(false);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
		psoDesc.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, 1);

		m_pGridPSO = m_pDevice->CreatePipeline(psoDesc);

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
			float padding2;
		};

		std::vector<InstanceData> instances;
		instances.reserve(EDITOR_GRID_INSTANCE_COUNT);

		for (int i = -EDITOR_GRID_INSTANCE_COUNT_HALF_SIZE; i < EDITOR_GRID_INSTANCE_COUNT_HALF_SIZE; ++i)
		{
			InstanceData& instanceData = instances.emplace_back();
			instanceData.Position = DirectX::XMFLOAT3(0.0f, 0.0f, static_cast<float>(i));
			instanceData.Color.R = 0.15f;
			instanceData.Color.G = 0.15f;
			instanceData.Color.B = 0.15f;

			if (i % 10 == 0)
			{
				instanceData.Color.R = 0.06f;
				instanceData.Color.G = 0.06f;
				instanceData.Color.B = 0.06f;
			}
		}

		m_pInstancesStructuredBuffer = m_pDevice->CreateBuffer(BufferDesc::CreateStructured(static_cast<uint32>(instances.size()), sizeof(InstanceData)), "Editor Grid Instances", instances.data());
	}

	void EditorGrid::Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept
	{
		RenderPassInfo info;
		info.RenderTargets[0].pTarget = sceneTextures.pColorTarget;
		info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargetCount++;

		info.DepthStencilTarget.pTarget = sceneTextures.pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ReadOnlyDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::None;

		commandContext.InsertResourceBarrier(sceneTextures.pColorTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandContext.InsertResourceBarrier(sceneTextures.pDepthTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		commandContext.BeginRenderPass(info);

		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		commandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());
		commandContext.SetPipelineState(m_pGridPSO);

		Renderer::BindViewData(commandContext, renderView);

		struct
		{
			Matrix BatchDataTransformVerticalMatrix = Matrix::Identity;
			Matrix BatchDataTransformHoriontalMatrix = Matrix::Identity;
			uint32 InstanceDataIndex = 0xFFFFFFFF;
		} params;

		DirectX::XMFLOAT3 offset = renderView.Location;
		offset.x = static_cast<float>(std::floor(offset.x - fmod(offset.x, 100.0)));
		offset.z = static_cast<float>(std::floor(offset.z - fmod(offset.z, 100.0)));

		{
			DirectX::XMMATRIX world = DirectX::XMMatrixScaling(10000.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(offset.x, 0.0f, 200.0f + offset.z);
			DirectX::XMStoreFloat4x4(&params.BatchDataTransformVerticalMatrix, world);
		}
		{
			DirectX::XMMATRIX world = DirectX::XMMatrixScaling(10000.0f, 1.0f, 1.0f) * DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(90.0f)) * DirectX::XMMatrixTranslation(offset.x - 200.0f, 0.0f, offset.z);
			DirectX::XMStoreFloat4x4(&params.BatchDataTransformHoriontalMatrix, world);
		}

		params.InstanceDataIndex = m_pInstancesStructuredBuffer->GetSRVIndex();

		commandContext.BindRootCBV(BindingSlot::PerPass, (const void*)&params, sizeof(params));

		commandContext.Draw(0, EDITOR_GRID_VERTEX_COUNT, 0, EDITOR_GRID_INSTANCE_COUNT);

		commandContext.EndRenderPass();

		commandContext.InsertResourceBarrier(sceneTextures.pColorTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
}