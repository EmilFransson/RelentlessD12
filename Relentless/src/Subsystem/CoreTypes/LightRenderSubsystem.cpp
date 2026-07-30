#include "LightRenderSubsystem.h"

#include "Graphics/RHI/Buffer.h"
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
			m_RenderData[renderProxy.ID].RenderProxy = renderProxy;
	}

	void LightRenderSubsystem::Remove(std::vector<uint32> someIDs) noexcept
	{
		for (auto& id : someIDs)
			m_RenderData.erase(id);
	}

	void LightRenderSubsystem::BuildDirectionalCascades(const RenderView& aRenderView, const LightRenderProxy& aLightRenderProxy, const std::vector<float>& someCascadeSplits) noexcept
	{
		RLS_ASSERT(aLightRenderProxy.LightType == ELightType::Directional, "[LightRenderSubsystem::BuildDirectionalCascades]: Light type mismatch.");
		
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
		
		const uint32 scaledResolution = DEFAULT_DIRECTIONAL_SHADOW_MAP_RESOLUTION * aLightRenderProxy.ShadowResolutionScale;
		const uint32 clampedResolution = Math::Min(Math::Max(scaledResolution, 1u), MAX_SHADOW_MAP_RESOLUTION);
		
		for (uint32 cascade = 0; cascade < someCascadeSplits.size(); ++cascade)
		{
			const float previousCascadeSplit = (cascade == 0u) ? minPoint : someCascadeSplits[cascade - 1];
			const float currentCascadeSplit = someCascadeSplits[cascade];
		
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
			Vector3 texelSize = extents / clampedResolution;
			minExtents = Math::Floor(minExtents / texelSize) * texelSize;
			maxExtents = Math::Floor(maxExtents / texelSize) * texelSize;
			center = (minExtents + maxExtents) * 0.5f;
		
			// Extent the Z bounds
			float extentsZ = fabs(center.z - minExtents.z);
			extentsZ = Math::Max(extentsZ, Math::Min(1500.0f, farPlane) * 0.5f);
			minExtents.z = center.z - extentsZ;
			maxExtents.z = center.z + extentsZ;

			const float texelWorldSize = (2.0f * radius) / float(clampedResolution);
			const Matrix projectionMatrix = Math::CreateOrthographicOffCenterMatrix(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, minExtents.z, maxExtents.z);

			CreateAndAddShadowView(aLightRenderProxy, lightView * projectionMatrix, clampedResolution, texelWorldSize, cascade);
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
		outLightData.Range = aRenderProxy.AttenuationRadius > 0.0f ? (1.0f / (aRenderProxy.AttenuationRadius * aRenderProxy.AttenuationRadius)) : 0.0f;
		outLightData.SpotlightAngles = Vector2(aRenderProxy.InnerConeAngle, aRenderProxy.OuterConeAngle);
		outLightData.IsEnabled = outLightData.Intensity > 0.0f;
		outLightData.CastShadows = aRenderProxy.CastShadows;
		outLightData.ChannelMask = static_cast<uint32>(aRenderProxy.ChannelMask);
	}

	void LightRenderSubsystem::BuildPointLightShadowView(const LightRenderProxy& aLightRenderProxy) noexcept
	{
		RLS_ASSERT(aLightRenderProxy.LightType == ELightType::Point, "[LightRenderSubsystem::BuildPointLightShadowView]: Light type mismatch.");

		const Matrix viewMatrices[] = 
		{
			Math::CreateLookToMatrix(aLightRenderProxy.Position, Vector3::Right,	Vector3::Up),
			Math::CreateLookToMatrix(aLightRenderProxy.Position, Vector3::Left,		Vector3::Up),
			Math::CreateLookToMatrix(aLightRenderProxy.Position, Vector3::Up,		Vector3::Forward),
			Math::CreateLookToMatrix(aLightRenderProxy.Position, Vector3::Down,		Vector3::Backward),
			Math::CreateLookToMatrix(aLightRenderProxy.Position, Vector3::Backward, Vector3::Up),
			Math::CreateLookToMatrix(aLightRenderProxy.Position, Vector3::Forward,	Vector3::Up),
		};
		const Matrix projection = Math::CreatePerspectiveMatrix(Math::PI_DIV_2, 1, aLightRenderProxy.AttenuationRadius, 0.01f);

		const uint32 scaledResolution = DEFAULT_POSITIONAL_SHADOW_MAP_RESOLUTION * aLightRenderProxy.ShadowResolutionScale;
		const uint32 clampedResolution = Math::Min(Math::Max(scaledResolution, 1u), MAX_SHADOW_MAP_RESOLUTION);
		const float frustumWidthAtFar = 2.0f * aLightRenderProxy.AttenuationRadius;
		const float texelWorldSize = frustumWidthAtFar / float(clampedResolution);

		for (uint64 face = 0u; face < ARRAYSIZE(viewMatrices); ++face)
			CreateAndAddShadowView(aLightRenderProxy, viewMatrices[face] * projection, clampedResolution, texelWorldSize, face);
	}

	void LightRenderSubsystem::BuildSpotLightShadowView(const LightRenderProxy& aLightRenderProxy) noexcept
	{
		RLS_ASSERT(aLightRenderProxy.LightType == ELightType::Spot, "[LightRenderSubsystem::BuildSpotLightShadowView]: Light type mismatch.");

		const Matrix lightView = Math::CreateLookToMatrix(aLightRenderProxy.Position, aLightRenderProxy.Direction, Vector3::Up);
		const Matrix projection = Math::CreatePerspectiveMatrix(aLightRenderProxy.OuterConeAngleRadians, 1.0f, aLightRenderProxy.AttenuationRadius, 0.01f);

		const uint32 scaledResolution = DEFAULT_POSITIONAL_SHADOW_MAP_RESOLUTION * aLightRenderProxy.ShadowResolutionScale;
		const uint32 clampedResolution = Math::Min(Math::Max(scaledResolution, 1u), MAX_SHADOW_MAP_RESOLUTION);
		const float frustumWidthAtFar = 2.0f * aLightRenderProxy.AttenuationRadius * tanf(0.5f * aLightRenderProxy.OuterConeAngleRadians);
		const float texelWorldSize = frustumWidthAtFar / float(clampedResolution);

		CreateAndAddShadowView(aLightRenderProxy, lightView * projection, clampedResolution, texelWorldSize, 0u);
	}

	void LightRenderSubsystem::BuildShadowViewData(ShaderInterop::ShadowViewData& outShadowViewData, const ShadowView& aShadowView) const noexcept
	{
		outShadowViewData.WorldToClip = aShadowView.WorldToClip;
		outShadowViewData.ShadowMapIndex = aShadowView.DepthTexture->GetSRVIndex();
		outShadowViewData.InverseShadowMapSize = 1.0f / static_cast<float>(aShadowView.DepthTexture->GetWidth());
		outShadowViewData.ShadowAmount = aShadowView.ShadowAmount;
		outShadowViewData.ConstantBiasWS = aShadowView.ConstantBiasWS;
		outShadowViewData.SlopeBiasWS = aShadowView.SlopeBiasWS;
	}

	std::vector<float> LightRenderSubsystem::ComputeCascadeSplits(const RenderView& aRenderView, uint32 aNumCascades, float aCascadeDistribution) const noexcept
	{
		const float nearPlane = Math::Min(aRenderView.NearPlane, aRenderView.FarPlane);
		const float farPlane = Math::Max(aRenderView.NearPlane, aRenderView.FarPlane);
		const float clipPlaneRange = farPlane - nearPlane;

		const float minZ = nearPlane;
		const float maxZ = nearPlane + clipPlaneRange;

		std::vector<float> cascadeSplits(aNumCascades);

		for (uint32 i = 0u; i < aNumCascades; ++i)
		{
			const float p = (i + 1u) / static_cast<float>(aNumCascades);
			const float log = minZ * std::pow(maxZ / minZ, p);
			const float uniform = minZ + (maxZ - minZ) * p;
			const float d = aCascadeDistribution * (log - uniform) + uniform;
			cascadeSplits[i] = (d - nearPlane) / clipPlaneRange;
		}

		return cascadeSplits;
	}

	void LightRenderSubsystem::CreateAndAddShadowView(const LightRenderProxy& aLightRenderProxy, const Matrix& aWorldToClipMatrix, uint32 aResolution, float aTexelWorldSize, uint32 aShadowMapIndex) noexcept
	{
		ShadowView shadowView;
		shadowView.IsPerspective = aLightRenderProxy.LightType != ELightType::Directional;
		shadowView.WorldToClip = aWorldToClipMatrix;
		shadowView.pRenderScene = m_pRenderScene;
		shadowView.pRenderer = m_pRenderScene->GetRenderer();
		shadowView.ShadowAmount = aLightRenderProxy.ShadowAmount;
		shadowView.ConstantBiasWS = aLightRenderProxy.ShadowBias * aTexelWorldSize;
		shadowView.SlopeBiasWS = aLightRenderProxy.ShadowSlopeBias * aTexelWorldSize;
		shadowView.LightChannels = aLightRenderProxy.ChannelMask;
		shadowView.Viewport = FloatRect(0.0f, 0.0f, static_cast<float>(aResolution), static_cast<float>(aResolution));

		std::vector<Ref<Texture>>& shadowMaps = m_RenderData[aLightRenderProxy.ID].ShadowMaps;
		if (aShadowMapIndex >= shadowMaps.size())
			shadowMaps.push_back(m_pGraphicsDevice->CreateTexture(TextureDesc::Create2D(aResolution, aResolution, ResourceFormat::R32_TYPELESS, 1, TextureFlag::DepthStencil | TextureFlag::ShaderResource, ClearBinding(0.0f, 0)), std::format("Shadow Map {} {}", aShadowMapIndex, aLightRenderProxy.ID).c_str()));
		else if (shadowMaps[aShadowMapIndex]->GetWidth() != aResolution)
			shadowMaps[aShadowMapIndex] = m_pGraphicsDevice->CreateTexture(TextureDesc::Create2D(aResolution, aResolution, ResourceFormat::R32_TYPELESS, 1, TextureFlag::DepthStencil | TextureFlag::ShaderResource, ClearBinding(0.0f, 0)), std::format("Shadow Map {} {}", aShadowMapIndex, aLightRenderProxy.ID).c_str());

		shadowView.DepthTexture = shadowMaps[aShadowMapIndex];
		
		m_ShadowViews.push_back(shadowView);
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

		const float nearPlane = Math::Min(aRenderView.NearPlane, aRenderView.FarPlane);
		const float farPlane = Math::Max(aRenderView.NearPlane, aRenderView.FarPlane);
		
		uint32 currentShadowViewBaseIndex = 0u;

		for (const auto& [index, renderData] : m_RenderData)
		{
			const LightRenderProxy& renderProxy = renderData.RenderProxy;

			if (!renderProxy.IsEnabled)
				continue;

			ShaderInterop::Light& lightData = m_LightCache.emplace_back();
			BuildLightData(lightData, renderProxy);

			if (!renderProxy.CastShadows)
				continue;

			if (renderProxy.LightType == ELightType::Directional)
			{
				const std::vector<float> cascadeSplits = ComputeCascadeSplits(aRenderView, renderProxy.NumCascades, renderProxy.CascadeDistribution);
				lightData.CascadeSplits = Vector4(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);

				const float clipRange = farPlane - nearPlane;
				for (uint32 i = 0; i < renderProxy.NumCascades; ++i)
				{
					const float viewSpaceSplit = nearPlane + cascadeSplits[i] * clipRange;
					Vector4& target = lightData.CascadeSplits;
					(&target.x)[i] = viewSpaceSplit;
				}

				lightData.ShadowViewBaseIndex = currentShadowViewBaseIndex;
				lightData.ShadowViewCount = renderProxy.NumCascades;
				
				BuildDirectionalCascades(aRenderView, renderProxy, cascadeSplits);
			}
			else if (renderProxy.LightType == ELightType::Spot)
			{
				lightData.ShadowViewBaseIndex = currentShadowViewBaseIndex;
				lightData.ShadowViewCount = 1u;
				BuildSpotLightShadowView(renderProxy);
			}
			else
			{
				lightData.ShadowViewBaseIndex = currentShadowViewBaseIndex;
				lightData.ShadowViewCount = 6u;
				BuildPointLightShadowView(renderProxy);
			}

			currentShadowViewBaseIndex += lightData.ShadowViewCount;
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