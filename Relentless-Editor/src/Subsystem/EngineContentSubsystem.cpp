#include "EngineContentSubsystem.h"

#include "Core/Editor.h"

namespace Relentless
{
	const AssetHandle& EngineContentSubsystem::GetCubeMeshHandle() const noexcept
	{
		RLS_ASSERT(m_CubeMesh.Handle.IsValid() && m_CubeMesh.Asset, "[EngineContentSubsystem::GetCubeMeshHandle]: Mesh is not loaded.");
		return m_CubeMesh.Handle;
	}

	const AssetHandle& EngineContentSubsystem::GetSphereMeshHandle() const noexcept
	{
		RLS_ASSERT(m_SphereMesh.Handle.IsValid() && m_SphereMesh.Asset, "[EngineContentSubsystem::GetSphereMeshHandle]: Mesh is not loaded.");
		return m_SphereMesh.Handle;
	}

	const AssetHandle& EngineContentSubsystem::GetMaterialPreviewCubemapHandle() const noexcept
	{
		RLS_ASSERT(m_MaterialPreviewTextureCube.Handle.IsValid() && m_MaterialPreviewTextureCube.Asset, "[EngineContentSubsystem::GetMaterialPreviewCubemap]: Texture is not loaded.");
		return m_MaterialPreviewTextureCube.Handle;
	}

	const AssetHandle& EngineContentSubsystem::GetNoneTexture2DHandle() const noexcept
	{
		RLS_ASSERT(m_NoneThumbnail.Handle.IsValid() && m_NoneThumbnail.Asset, "[EngineContentSubsystem::GetNoneTexture2DHandle]: Texture is not loaded.");
		return m_NoneThumbnail.Handle;
	}

	const AssetHandle& EngineContentSubsystem::GetWhiteMaterialHandle() const noexcept
	{
		RLS_ASSERT(m_DefaultWhiteMaterial.Handle.IsValid() && m_DefaultWhiteMaterial.Asset, "[EngineContentSubsystem::GetWhiteMaterialHandle]: Material is not loaded.");
		return m_DefaultWhiteMaterial.Handle;
	}

	const AssetHandle& EngineContentSubsystem::GetCitrusOrchardRoadTextureCubeHandle() const noexcept
	{
		RLS_ASSERT(m_CitrusOrchardRoadPureSkyTextureCube.Handle.IsValid() && m_CitrusOrchardRoadPureSkyTextureCube.Asset, "[EngineContentSubsystem::GetCitrusOrchardRoadTextureCubeHandle]: Texture is not loaded.");
		return m_CitrusOrchardRoadPureSkyTextureCube.Handle;
	}

	const AssetHandle& EngineContentSubsystem::GetOvercastSoilTextureCubeHandle() const noexcept
	{
		RLS_ASSERT(m_OvercastSoilPureSkyTextureCube.Handle.IsValid() && m_OvercastSoilPureSkyTextureCube.Asset, "[EngineContentSubsystem::GetOvercastSoilTextureCubeHandle]: Texture is not loaded.");
		return m_OvercastSoilPureSkyTextureCube.Handle;
	}

	const AssetHandle& EngineContentSubsystem::GetCitrusOrchardRoadEnvironmentHandle() const noexcept
	{
		RLS_ASSERT(m_CitrusOrchardRoadPureSkyEnvironment.Handle.IsValid() && m_CitrusOrchardRoadPureSkyEnvironment.Asset, "[EngineContentSubsystem::GetCitrusOrchardRoadEnvironmentHandle]: Environment is not loaded.");
		return m_CitrusOrchardRoadPureSkyEnvironment.Handle;
	}

	const AssetHandle& EngineContentSubsystem::GetOvercastSoilEnvironmentHandle() const noexcept
	{
		RLS_ASSERT(m_OvercastSoilPureSkyEnvironment.Handle.IsValid() && m_OvercastSoilPureSkyEnvironment.Asset, "[EngineContentSubsystem::GetOvercastSoilEnvironmentHandle]: Environment is not loaded.");
		return m_OvercastSoilPureSkyEnvironment.Handle;
	}

	bool EngineContentSubsystem::IsLoading() const noexcept
	{
		return m_NumAssetsLoading > 0u;
	}

	bool EngineContentSubsystem::OnLoad(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept
	{
		RequestAsyncLoad("Engine/Models/Cube", m_CubeMesh);
		RequestAsyncLoad("Engine/Models/Sphere", m_SphereMesh);
		RequestAsyncLoad("Engine/Textures/quattro_canti_2k", m_MaterialPreviewTextureCube);
		RequestAsyncLoad("Engine/Textures/citrus_orchard_road_puresky_2k", m_CitrusOrchardRoadPureSkyTextureCube);
		RequestAsyncLoad("Engine/Textures/overcast_soil_puresky_2k", m_OvercastSoilPureSkyTextureCube);
		RequestAsyncLoad("Engine/Textures/none_thumbnail", m_NoneThumbnail);
		RequestAsyncLoad("Engine/Materials/M_DefaultWhite", m_DefaultWhiteMaterial);
		RequestAsyncLoad("Engine/Environments/CitrusOrchardRoadEnvironment", m_CitrusOrchardRoadPureSkyEnvironment);
		RequestAsyncLoad("Engine/Environments/OvercastSoilEnvironment", m_OvercastSoilPureSkyEnvironment);

		return true;
	}

	bool EngineContentSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}
}