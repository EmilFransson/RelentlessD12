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
		RLS_ASSERT(m_MaterialPreviewEnvironment.Handle.IsValid() && m_MaterialPreviewEnvironment.Asset, "[EngineContentSubsystem::GetMaterialPreviewCubemap]: Texture is not loaded.");
		return m_MaterialPreviewEnvironment.Handle;
	}

	const AssetHandle& EngineContentSubsystem::GetNoneTexture2DHandle() const noexcept
	{
		RLS_ASSERT(m_NoneThumbnail.Handle.IsValid() && m_NoneThumbnail.Asset, "[EngineContentSubsystem::GetNoneTexture2DHandle]: Texture is not loaded.");
		return m_NoneThumbnail.Handle;
	}

	bool EngineContentSubsystem::IsLoading() const noexcept
	{
		return m_NumAssetsLoading > 0u;
	}

	bool EngineContentSubsystem::OnLoad(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept
	{
		RequestAsyncLoad("Engine/Models/Cube", m_CubeMesh);
		RequestAsyncLoad("Engine/Models/Sphere", m_SphereMesh);
		RequestAsyncLoad("Engine/Textures/quattro_canti_4k", m_MaterialPreviewEnvironment);
		RequestAsyncLoad("Engine/Textures/none_thumbnail", m_NoneThumbnail);

		return true;
	}

	bool EngineContentSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}
}