#include "LightRenderSubsystem.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Scene/RenderScene.h"

namespace Relentless
{
	uint32 LightRenderSubsystem::GetNumLights() const noexcept
	{
		return m_LightDataBuffer.Count;
	}

	const Buffer* LightRenderSubsystem::GetLightRenderData() const noexcept
	{
		RLS_ASSERT(m_LightDataBuffer.pBuffer, "[LightRenderSubsystem::GetLightRenderData]: Buffer is invalid.");
		return m_LightDataBuffer.pBuffer;
	}

	const std::vector<ShadowView>& LightRenderSubsystem::GetShadowViews() const noexcept
	{
		return m_ShadowViews;
	}

	const Buffer* LightRenderSubsystem::GetShadowViewsRenderData() const noexcept
	{
		RLS_ASSERT(m_ShadowViewDataBuffer.pBuffer, "[LightRenderSubsystem::GetShadowViewsRenderData]: Buffer is invalid.");
		return m_ShadowViewDataBuffer.pBuffer;
	}

	bool LightRenderSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		m_pRenderScene = static_cast<RenderScene*>(aSystemManager);
		m_pRenderScene->OnViewPrepare.Connect(this, &LightRenderSubsystem::OnViewPrepare);

		Renderer* pRenderer = m_pRenderScene->GetRenderer();
		m_OnUploadCallbackID = pRenderer->RegisterOnUploadCallback(Callback<void(CommandContext&)>::Bind(this, &LightRenderSubsystem::OnUpload));

		m_pGraphicsDevice = pRenderer->GetDevice();

		return true;
	}

	void LightRenderSubsystem::OnUnload(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);
		pRenderScene->OnViewPrepare.Detach(this);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		pRenderer->UnregisterOnUploadCallback(m_OnUploadCallbackID);
	}

	bool LightRenderSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<RenderScene*>(aSystemManager) != nullptr;
	}

	void LightRenderSubsystem::Patch(std::vector<LightRenderProxy> someRenderProxyUpdates) noexcept
	{
		for (auto& renderProxy : someRenderProxyUpdates)
			m_RenderData[renderProxy.ID] = renderProxy;
	}

	void LightRenderSubsystem::Remove(std::vector<uint32> someIDs) noexcept
	{
		for (auto& id : someIDs)
			m_RenderData.erase(id);
	}

	void LightRenderSubsystem::BuildDirectionalCascades(const RenderView& aRenderView, const LightRenderProxy& aLightRenderProxy, const std::array<float, MAX_CASCADES>& someCascadeSplits, uint32& aShadowViewIndex) noexcept
	{
		RLS_ASSERT(aLightRenderProxy.LightType == ELightType::Directional, "[LightRenderSubsystem::BuildDirectionalCascades]: Light is not directional.");

		auto AddShadowView = [&](ShadowView shadowView, uint32 resolution)
			{
				if (aShadowViewIndex >= (uint32)m_ShadowMaps.size())
					m_ShadowMaps.push_back(m_pGraphicsDevice->CreateTexture(TextureDesc::Create2D(resolution, resolution, ResourceFormat::R16_TYPELESS, 1, TextureFlag::DepthStencil | TextureFlag::ShaderResource, ClearBinding(1.0f, 0)), std::format("Shadow Map {}", (uint32)m_ShadowMaps.size()).c_str()));
				Ref<Texture> pTarget = m_ShadowMaps[aShadowViewIndex];

				shadowView.DepthTexture = pTarget;
				shadowView.Viewport = FloatRect(0.0f, 0.0f, (float)resolution, (float)resolution);
				m_ShadowViews.push_back(shadowView);

				aShadowViewIndex++;
			};
		
		const Matrix vpInverse = aRenderView.WorldToClip.Invert();
		
		const Vector3 frustumCornersWS[] = 
		{
			Vector3::Transform(Vector3(-1, -1, 1), vpInverse),
			Vector3::Transform(Vector3(-1, -1, 0), vpInverse),
			Vector3::Transform(Vector3(-1, 1, 1), vpInverse),
			Vector3::Transform(Vector3(-1, 1, 0), vpInverse),
			Vector3::Transform(Vector3(1, 1, 1), vpInverse),
			Vector3::Transform(Vector3(1, 1, 0), vpInverse),
			Vector3::Transform(Vector3(1, -1, 1), vpInverse),
			Vector3::Transform(Vector3(1, -1, 0), vpInverse),
		};

		const Matrix lightView = aLightRenderProxy.WorldMatrix.Invert();
		const float farPlane = Math::Max(aRenderView.NearPlane, aRenderView.FarPlane);
		constexpr float minPoint = 0.0f;
		
		for (uint32 i = 0; i < someCascadeSplits.size(); ++i)
		{
			const float previousCascadeSplit = (i == 0u) ? minPoint : someCascadeSplits[i - 1];
			const float currentCascadeSplit = someCascadeSplits[i];

			// Compute the frustum corners for the cascade in view space
			const Vector3 cornersVS[] =
			{
				Vector3::Transform(Vector3::Lerp(frustumCornersWS[0], frustumCornersWS[1], previousCascadeSplit), lightView),
				Vector3::Transform(Vector3::Lerp(frustumCornersWS[0], frustumCornersWS[1], currentCascadeSplit), lightView),
				Vector3::Transform(Vector3::Lerp(frustumCornersWS[2], frustumCornersWS[3], previousCascadeSplit), lightView),
				Vector3::Transform(Vector3::Lerp(frustumCornersWS[2], frustumCornersWS[3], currentCascadeSplit), lightView),
				Vector3::Transform(Vector3::Lerp(frustumCornersWS[4], frustumCornersWS[5], previousCascadeSplit), lightView),
				Vector3::Transform(Vector3::Lerp(frustumCornersWS[4], frustumCornersWS[5], currentCascadeSplit), lightView),
				Vector3::Transform(Vector3::Lerp(frustumCornersWS[6], frustumCornersWS[7], previousCascadeSplit), lightView),
				Vector3::Transform(Vector3::Lerp(frustumCornersWS[6], frustumCornersWS[7], currentCascadeSplit), lightView),
			};

			Vector3 center = Vector3::Zero;
			for (const Vector3& corner : cornersVS)
				center += corner;
			center /= ARRAYSIZE(cornersVS);

			//Create a bounding sphere to maintain aspect in projection to avoid flickering when rotating
			float radius = 0;
			for (const Vector3& corner : cornersVS)
			{
				float dist = Vector3::Distance(center, corner);
				radius = Math::Max(dist, radius);
			}
			Vector3 minExtents = center - Vector3(radius);
			Vector3 maxExtents = center + Vector3(radius);

			// Snap the cascade to the resolution of the shadow map
			Vector3 extents = maxExtents - minExtents;
			Vector3 texelSize = extents / 2048;
			minExtents = Math::Floor(minExtents / texelSize) * texelSize;
			maxExtents = Math::Floor(maxExtents / texelSize) * texelSize;
			center = (minExtents + maxExtents) * 0.5f;

			// Extent the Z bounds
			float extentsZ = fabs(center.z - minExtents.z);
			extentsZ = Math::Max(extentsZ, Math::Min(1500.0f, farPlane) * 0.5f);
			minExtents.z = center.z - extentsZ;
			maxExtents.z = center.z + extentsZ;

			const Matrix projectionMatrix = Math::CreateOrthographicOffCenterMatrix(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, minExtents.z, maxExtents.z);

			ShadowView shadowView{};
			shadowView.IsPerspective = false;
			shadowView.WorldToClip = lightView * projectionMatrix;
			shadowView.WorldToClipPrev = shadowView.WorldToClip;
			shadowView.OrthographicFrustum.Center = center;
			shadowView.OrthographicFrustum.Extents = maxExtents - minExtents;
			shadowView.OrthographicFrustum.Extents.z *= 10.0f;
			shadowView.OrthographicFrustum.Orientation = Quaternion::CreateFromRotationMatrix(lightView.Invert());
			shadowView.pRenderScene = m_pRenderScene;
			shadowView.pRenderer = m_pRenderScene->GetRenderer();

			AddShadowView(shadowView, 2048u);
		}
	}

	void LightRenderSubsystem::BuildLightData(ShaderInterop::Light& outLightData, const LightRenderProxy& aRenderProxy) const noexcept
	{
		outLightData.Color = aRenderProxy.Color;
		outLightData.Position = aRenderProxy.Position;
		outLightData.Direction = aRenderProxy.Direction;
		outLightData.Intensity = aRenderProxy.Intensity;
		outLightData.IsDirectional = aRenderProxy.LightType == ELightType::Directional;
		outLightData.IsPoint = aRenderProxy.LightType == ELightType::Point;
		outLightData.IsSpot = aRenderProxy.LightType == ELightType::Spot;
		outLightData.Range = aRenderProxy.AttenuationRadius;
		outLightData.SpotlightAngles = Vector2(aRenderProxy.InnerConeAngle, aRenderProxy.OuterConeAngle);
		outLightData.IsEnabled = outLightData.Intensity > 0.0f;
	}

	void LightRenderSubsystem::BuildShadowViewData(ShaderInterop::ShadowViewData& outShadowViewData, const ShadowView& aShadowView) const noexcept
	{
		outShadowViewData.WorldToClip = aShadowView.WorldToClip;
		outShadowViewData.ShadowMapIndex = aShadowView.DepthTexture->GetSRVIndex();

		//TODO: Possibly more fields. (what about e.g. frustum splits etc....)
	}

	std::array<float, LightRenderSubsystem::MAX_CASCADES> LightRenderSubsystem::ComputeCascadeSplits(const RenderView& aRenderView) const noexcept
	{
		const float nearPlane = Math::Min(aRenderView.NearPlane, aRenderView.FarPlane);
		const float farPlane = Math::Max(aRenderView.NearPlane, aRenderView.FarPlane);
		const float clipPlaneRange = farPlane - nearPlane;

		const float minPoint = 0.0f;
		const float maxPoint = 1.0f;

		const float minZ = nearPlane + minPoint * clipPlaneRange;
		const float maxZ = nearPlane + maxPoint * clipPlaneRange;
		constexpr float pssmLambda = 0.85f;

		std::array<float, MAX_CASCADES> cascadeSplits{};

		for (uint32 i = 0u; i < MAX_CASCADES; ++i)
		{
			const float p = (i + 1u) / static_cast<float>(MAX_CASCADES);
			const float log = minZ * std::pow(maxZ / minZ, p);
			const float uniform = minZ + (maxZ - minZ) * p;
			const float d = pssmLambda * (log - uniform) + uniform;
			cascadeSplits[i] = (d - nearPlane) / clipPlaneRange;
		}

		return cascadeSplits;
	}

	void LightRenderSubsystem::OnUpload(CommandContext& aCommandContext) noexcept
	{
		auto CopyBufferData = [&](uint32 numElements, uint32 stride, const char* pName, const void* pSource, SceneBuffer& target)
			{
				uint32 desiredElements = Math::AlignUp(Math::Max(1u, numElements), 8u);
				if (!target.pBuffer || desiredElements > target.pBuffer->GetNrOfElements())
					target.pBuffer = m_pGraphicsDevice->CreateBuffer(BufferDesc::CreateStructured(desiredElements, stride, BufferFlag::ShaderResource), pName);
				ScratchAllocation alloc = aCommandContext.AllocateScratch(numElements * stride);
				memcpy(alloc.pMappedMemory, pSource, numElements * stride);
				aCommandContext.CopyBuffer(alloc.pBackingResource, target.pBuffer, alloc.Size, alloc.Offset, 0);
				target.Count = numElements;
			};

		CopyBufferData(static_cast<uint32>(m_LightCache.size()), sizeof(ShaderInterop::Light), "LightData", m_LightCache.data(), m_LightDataBuffer);
		CopyBufferData(static_cast<uint32>(m_ShadowViewDataCache.size()), sizeof(ShaderInterop::ShadowViewData), "Shadow View Data", m_ShadowViewDataCache.data(), m_ShadowViewDataBuffer);
	}

	void LightRenderSubsystem::OnViewPrepare(RenderView& aRenderView) noexcept
	{
		m_ShadowViews.clear();
		m_LightCache.clear();
		m_LightCache.reserve(m_RenderData.size());

		const std::array<float, MAX_CASCADES> cascadeSplits = ComputeCascadeSplits(aRenderView);
		uint32 currentShadowViewBaseIndex = 0u;

		const float nearPlane = Math::Min(aRenderView.NearPlane, aRenderView.FarPlane);
		const float farPlane = Math::Max(aRenderView.NearPlane, aRenderView.FarPlane);

		for (const auto& [index, renderProxy] : m_RenderData)
		{
			if (!renderProxy.IsEnabled)
				continue;

			ShaderInterop::Light& lightData = m_LightCache.emplace_back();
			BuildLightData(lightData, renderProxy);

			if (renderProxy.LightType == ELightType::Directional)
			{
				lightData.CascadeSplits = Vector4
				(
					nearPlane + cascadeSplits[0] * (farPlane - nearPlane),
					nearPlane + cascadeSplits[1] * (farPlane - nearPlane),
					nearPlane + cascadeSplits[2] * (farPlane - nearPlane),
					nearPlane + cascadeSplits[3] * (farPlane - nearPlane)
				);

				lightData.ShadowViewBaseIndex = currentShadowViewBaseIndex;
				lightData.ShadowViewCount = MAX_CASCADES;
				
				BuildDirectionalCascades(aRenderView, renderProxy, cascadeSplits, currentShadowViewBaseIndex);
			}
		}

		m_ShadowViewDataCache.clear();
		m_ShadowViewDataCache.reserve(m_ShadowViews.size());

		for (const ShadowView& view : m_ShadowViews)
		{
			ShaderInterop::ShadowViewData& shadowViewData = m_ShadowViewDataCache.emplace_back();
			BuildShadowViewData(shadowViewData, view);
		}	
	}
}