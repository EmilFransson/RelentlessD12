#include "RenderBakeService.h"

#include "Core/Application.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"

namespace Relentless
{
	RenderBakeService::RenderBakeService() noexcept
	{
		GraphicsDevice* pDevice = Application::Get().GetGraphicsDevice();

		PipelineStateInitializer psoDesc{};
		psoDesc.SetDepthWrite(false);
		psoDesc.SetDepthEnabled(false);
		psoDesc.SetName("Equirectangular To Cubemap");
		psoDesc.SetRootSignature(pDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, 1);
		psoDesc.SetVertexShader("EquirectangularToCubemapShader", "vs_main");
		psoDesc.SetPixelShader("EquirectangularToCubemapShader", "ps_main");

		m_pEquirectToCubemapPSO = pDevice->CreatePipeline(psoDesc);
	}

	RenderBakeService::~RenderBakeService() noexcept = default;

	RenderJobHandle RenderBakeService::RequestEquirectangularToCubemapConversion(EquirectangularToCubemapSpecification& aSpecification, Ref<Texture>& aOutCubemap) noexcept
	{
		return Renderer::SubmitRenderJob([this, aSpecification, &aOutCubemap](CommandContext& aCommandContext) mutable
			{
				GraphicsDevice* pDevice = Application::Get().GetGraphicsDevice();

				Ref<Texture> pTextureCube = pDevice->CreateTexture(TextureDesc::CreateCube(aSpecification.CubeFaceDimension, aSpecification.CubeFaceDimension, ResourceFormat::RGBA32_FLOAT, 1u, TextureFlag::RenderTarget | TextureFlag::ShaderResource), "Temp_Name");

				const Matrix viewMatrices[] = 
				{
					Math::CreateLookToMatrix(Vector3::Zero, Vector3::Right,		Vector3::Up),
					Math::CreateLookToMatrix(Vector3::Zero, Vector3::Left,		Vector3::Up),
					Math::CreateLookToMatrix(Vector3::Zero, Vector3::Up,		Vector3::Forward),
					Math::CreateLookToMatrix(Vector3::Zero, Vector3::Down,		Vector3::Backward),
					Math::CreateLookToMatrix(Vector3::Zero, Vector3::Backward,	Vector3::Up),
					Math::CreateLookToMatrix(Vector3::Zero, Vector3::Forward,	Vector3::Up),
				};

				const Matrix projection = Math::CreatePerspectiveMatrix(Math::PI_DIV_2, 1.0f, 0.01f, 1'000.0f);

				struct
				{
					Matrix ViewProjectionMatrix = Matrix::Identity;
					uint32 EquirectangularTextureIndex = 0u;
					float Padding[3];
				} passData;

				passData.EquirectangularTextureIndex = aSpecification.EquirectangularTexture->GetSRVIndex();

				for (uint32 face = 0u; face < 6; ++face)
				{
					passData.ViewProjectionMatrix = viewMatrices[face] * projection;

					RenderPassInfo renderPassInfo;
					renderPassInfo.RenderTargets[0].pTarget = pTextureCube;
					renderPassInfo.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Preserve;
					renderPassInfo.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
					renderPassInfo.RenderTargets[0].ArrayIndex = face;
					renderPassInfo.RenderTargets[0].MipLevel = 0;
					renderPassInfo.RenderTargetCount++;

					aCommandContext.BeginRenderPass(renderPassInfo);
					
					aCommandContext.SetGraphicsRootSignature(pDevice->GetGlobalRootSignature());
					aCommandContext.SetPipelineState(m_pEquirectToCubemapPSO);
					aCommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

					aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&passData, sizeof(passData));
					aCommandContext.Draw(0u, 36u, 0u, 1u);
					
					aCommandContext.EndRenderPass();
				}

				aOutCubemap = pTextureCube;
			});
	}
}