#include "ExponentialHeightFogComponent.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/TextureCube.h"

#include "Scene/Scene.h"

namespace Relentless
{
	ExponentialHeightFogComponent::ExponentialHeightFogComponent(ExponentialHeightFogComponent&& aOther) noexcept
		: ManagedComponent<ExponentialHeightFogComponent>(std::move(aOther))
		, m_InscatterCubemapHandle(aOther.m_InscatterCubemapHandle)
	{
		aOther.DetachTextureCube();
		aOther.m_InscatterCubemapHandle = AssetHandle::INVALID;
		ConnectTextureCube();
	}

	ExponentialHeightFogComponent& ExponentialHeightFogComponent::operator=(ExponentialHeightFogComponent&& aOther) noexcept
	{
		if (this != &aOther)
		{
			DetachTextureCube();
			aOther.DetachTextureCube();
			m_InscatterCubemapHandle = std::move(aOther.m_InscatterCubemapHandle);
			aOther.m_InscatterCubemapHandle = AssetHandle::INVALID;
			ConnectTextureCube();
		}
		return *this;
	}

	ExponentialHeightFogComponent::~ExponentialHeightFogComponent() noexcept
	{
		DetachTextureCube();
	}

	void ExponentialHeightFogComponent::CopyFrom(const ExponentialHeightFogComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		DetachTextureCube();
		m_InscatterCubemapHandle = aOtherComponent.m_InscatterCubemapHandle;
		ConnectTextureCube();
		
		m_FogLayers = aOtherComponent.m_FogLayers;
		m_InscatteringColor = aOtherComponent.m_InscatteringColor;
		m_MaxOpacity = aOtherComponent.m_MaxOpacity;
		m_InscatterMode = aOtherComponent.m_InscatterMode;

		this->m_Self = aThisEntity;
		this->m_EntityManager = &aEntityManager;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
	}

	const ExponentialHeightFogComponent::FogLayer& ExponentialHeightFogComponent::GetFogLayer(uint8 aLayerIndex) const noexcept
	{
		RLS_ASSERT(aLayerIndex < NUM_FOG_LAYERS, "[ExponentialHeightFogComponent::GetFogLayer]: Invalid layer index.");
		return m_FogLayers[aLayerIndex];
	}

	float ExponentialHeightFogComponent::GetFullyDirectionalInScatteringColorDistance() const noexcept
	{
		return m_FullyDirectionalInScatteringColorDistance;
	}

	const Color& ExponentialHeightFogComponent::GetInscatteringColor() const noexcept
	{
		return m_InscatteringColor;
	}

	float ExponentialHeightFogComponent::GetInscatteringColorIntensity() const noexcept
	{
		return m_InscatteringColorIntensity;
	}

	EFogInscatterMode ExponentialHeightFogComponent::GetInscatterMode() const noexcept
	{
		return m_InscatterMode;
	}

	Ref<TextureCube> ExponentialHeightFogComponent::GetInscatterTexture() const noexcept
	{
		RLS_ASSERT(m_InscatterCubemapHandle.IsValid(), "[ExponentialHeightFogComponent::GetInscatterTexture]: Invalid cubemap asset handle.");
		return AssetManager::Get<TextureCube>(m_InscatterCubemapHandle);
	}

	const AssetHandle& ExponentialHeightFogComponent::GetInscatterTextureHandle() noexcept
	{
		return m_InscatterCubemapHandle;
	}

	const Color& ExponentialHeightFogComponent::GetInscatterTextureTintColor() const noexcept
	{
		return m_InscatteringTextureTintColor;
	}

	float ExponentialHeightFogComponent::GetMaxOpacity() const noexcept
	{
		return m_MaxOpacity;
	}

	float ExponentialHeightFogComponent::GetNonDirectionalInScatteringColorDistance() const noexcept
	{
		return m_NonDirectionalInScatteringColorDistance;
	}

	bool ExponentialHeightFogComponent::HasAssignedInScatterTexture() const noexcept
	{
		return m_InscatterCubemapHandle.IsValid();
	}

	void ExponentialHeightFogComponent::OnBound() noexcept
	{
		m_FogLayers[1].Density = 0.0f;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
	}

	void ExponentialHeightFogComponent::RemoveInscatterTexture() noexcept
	{
		if (!m_InscatterCubemapHandle.IsValid())
			return;

		m_InscatterCubemapHandle = AssetHandle::INVALID;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_InscatterCubemapHandle);
	}

	void ExponentialHeightFogComponent::SetFullyDirectionalInScatteringColorDistance(float aDistance) noexcept
	{
		if (Math::AreValuesClose(m_FullyDirectionalInScatteringColorDistance, aDistance))
			return;

		m_FullyDirectionalInScatteringColorDistance = aDistance;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_FullyDirectionalInScatteringColorDistance);
	}

	void ExponentialHeightFogComponent::SetInscatteringColor(const Color& aColor) noexcept
	{
		if (m_InscatteringColor == aColor)
			return;

		m_InscatteringColor = aColor;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_InscatteringColor);	
	}

	void ExponentialHeightFogComponent::SetInscatteringColorIntensity(float aIntensity) noexcept
	{
		if (Math::AreValuesClose(m_InscatteringColorIntensity, aIntensity))
			return;

		m_InscatteringColorIntensity = aIntensity;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_InscatteringColorIntensity);
	}

	void ExponentialHeightFogComponent::SetInscatterMode(EFogInscatterMode aInscatterMode) noexcept
	{
		if (m_InscatterMode == aInscatterMode)
			return;

		m_InscatterMode = aInscatterMode;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_InscatterMode);
	}

	void ExponentialHeightFogComponent::SetInscatterTexture(const AssetHandle& aCubemapAssetHandle) noexcept
	{
		RLS_ASSERT(aCubemapAssetHandle.Type == TextureCube::StaticType(), "[ExponentialHeightFogComponent::SetInscatterTexture]: Invalid cubemap asset handle.");

		if (m_InscatterCubemapHandle == aCubemapAssetHandle)
			return;

		m_InscatterCubemapHandle = aCubemapAssetHandle;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_InscatterCubemapHandle);
	}

	void ExponentialHeightFogComponent::SetInscatterTextureTintColor(const Color& aTintColor) noexcept
	{
		if (m_InscatteringTextureTintColor == aTintColor)
			return;

		m_InscatteringTextureTintColor = aTintColor;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_InscatteringTextureTintColor);
	}

	void ExponentialHeightFogComponent::SetLayerDensity(uint8 aLayerIndex, float aDensity) noexcept
	{
		RLS_ASSERT(aLayerIndex < NUM_FOG_LAYERS, "[ExponentialHeightFogComponent::SetLayerDensity]: Invalid layer index.");

		FogLayer& fogLayer = m_FogLayers[aLayerIndex];
		if (Math::AreValuesClose(fogLayer.Density, aDensity))
			return;

		fogLayer.Density = aDensity;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_FogLayers);
	}

	void ExponentialHeightFogComponent::SetLayerHeightFalloff(uint8 aLayerIndex, float aHeightFalloff) noexcept
	{
		RLS_ASSERT(aLayerIndex < NUM_FOG_LAYERS, "[ExponentialHeightFogComponent::SetLayerHeightFalloff]: Invalid layer index.");

		FogLayer& fogLayer = m_FogLayers[aLayerIndex];
		if (Math::AreValuesClose(fogLayer.HeightFalloff, aHeightFalloff))
			return;

		fogLayer.HeightFalloff = aHeightFalloff;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_FogLayers);
	}

	void ExponentialHeightFogComponent::SetLayerEndDistance(uint8 aLayerIndex, float aEndDistance) noexcept
	{
		RLS_ASSERT(aLayerIndex < NUM_FOG_LAYERS, "[ExponentialHeightFogComponent::SetLayerEndDistance]: Invalid layer index.");

		FogLayer& fogLayer = m_FogLayers[aLayerIndex];
		if (Math::AreValuesClose(fogLayer.EndDistance, aEndDistance))
			return;

		fogLayer.EndDistance = aEndDistance;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_FogLayers);
	}

	void ExponentialHeightFogComponent::SetLayerStartDistance(uint8 aLayerIndex, float aStartDistance) noexcept
	{
		RLS_ASSERT(aLayerIndex < NUM_FOG_LAYERS, "[ExponentialHeightFogComponent::SetLayerStartDistance]: Invalid layer index.");

		FogLayer& fogLayer = m_FogLayers[aLayerIndex];
		if (Math::AreValuesClose(fogLayer.StartDistance, aStartDistance))
			return;

		fogLayer.StartDistance = aStartDistance;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_FogLayers);
	}

	void ExponentialHeightFogComponent::SetLayerHeightOffset(uint8 aLayerIndex, float aHeightOffset) noexcept
	{
		RLS_ASSERT(aLayerIndex < NUM_FOG_LAYERS, "[ExponentialHeightFogComponent::SetLayerHeightOffset]: Invalid layer index.");

		FogLayer& fogLayer = m_FogLayers[aLayerIndex];
		if (Math::AreValuesClose(fogLayer.HeightOffset, aHeightOffset))
			return;

		fogLayer.HeightOffset = aHeightOffset;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_FogLayers);
	}

	void ExponentialHeightFogComponent::SetMaxOpacity(float aMaxOpacity) noexcept
	{
		if (Math::AreValuesClose(m_MaxOpacity, aMaxOpacity))
			return;

		m_MaxOpacity = aMaxOpacity;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_MaxOpacity);
	}

	void ExponentialHeightFogComponent::SetNonDirectionalInScatteringColorDistance(float aDistance) noexcept
	{
		if (Math::AreValuesClose(m_NonDirectionalInScatteringColorDistance, aDistance))
			return;

		m_NonDirectionalInScatteringColorDistance = aDistance;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_NonDirectionalInScatteringColorDistance);
	}

	void ExponentialHeightFogComponent::ConnectTextureCube() noexcept
	{
		if (!m_InscatterCubemapHandle.IsValid())
			return;

		Ref<TextureCube> pTextureCube = AssetManager::Get<TextureCube>(m_InscatterCubemapHandle);
		pTextureCube->OnDestroy.Connect(this, &ExponentialHeightFogComponent::OnTextureCubeAssetDestroy);
		pTextureCube->OnPropertyChanged.Connect(this, &ExponentialHeightFogComponent::OnTextureCubeAssetPropertyChanged);
	}

	void ExponentialHeightFogComponent::DetachTextureCube() noexcept
	{
		if (!m_InscatterCubemapHandle.IsValid())
			return;

		Ref<TextureCube> pTextureCube = AssetManager::Get<TextureCube>(m_InscatterCubemapHandle);
		pTextureCube->OnDestroy.Detach(this);
		pTextureCube->OnPropertyChanged.Detach(this);
	}

	void ExponentialHeightFogComponent::OnTextureCubeAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept
	{
		RemoveInscatterTexture();
	}

	void ExponentialHeightFogComponent::OnTextureCubeAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}
}