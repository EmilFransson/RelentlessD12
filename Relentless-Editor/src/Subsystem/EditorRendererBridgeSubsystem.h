#pragma once
#include <Relentless.h>

namespace Relentless
{
	class Editor;

	class EditorRendererBridgeSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD entity GetHoveredEntity() const noexcept;

		void OnEntityReadbackDone(uint32 aEntityID) noexcept;
		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		NO_DISCARD AssetHandle OnRequestBRDFLut() noexcept;
		
		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	private:
		AssetHandle m_BRDFLutTextureHandle = AssetHandle::INVALID;
		entity m_HoveredEntity = NULL_ENTITY;
		Editor* m_pEditor = nullptr;
	};
}