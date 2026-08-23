#include "EngineContentSubsystem.h"

#include "Core/Editor.h"

namespace Relentless
{
	bool EngineContentSubsystem::IsLoading() const noexcept
	{
		return m_NumAssetsLoading > 0u;
	}

	bool EngineContentSubsystem::OnLoad(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept
	{
		#define RLS_ENGINE_ASSET(InId, InType, InPath) \
		RequestAsyncLoad<InType>(InPath, m_Assets[(uint32)EEngineAsset::InId]);
		#include "EngineContent.inl"
		#undef RLS_ENGINE_ASSET

		//RequestAsyncLoad("/Engine/Textures/quattro_canti_2k", m_MaterialPreviewTextureCube);

		return true;
	}

	bool EngineContentSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}
}