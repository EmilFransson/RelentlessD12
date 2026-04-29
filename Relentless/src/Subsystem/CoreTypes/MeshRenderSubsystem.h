#pragma once
#include "Subsystem/ISubsystem.h"

#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RenderProxy/MeshRenderProxy.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	class RLS_API MeshRenderSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD uint32 GetNumMeshes() const noexcept;
		NO_DISCARD const Buffer* GetRenderData() const noexcept;
		NO_DISCARD uint32 GetSlotIndex(const UUID& aUUID) const noexcept;
		NO_DISCARD const MeshRenderProxy& GetProxy(const UUID& aUUID) const noexcept;

		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		void OnUnload(ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		void Patch(std::vector<MeshRenderProxy> someRenderProxyUpdates) noexcept;

		void Remove(std::vector<UUID> someIDs) noexcept;
	private:
		void BuildMeshData(ShaderInterop::MeshData& outMeshData, const MeshRenderProxy& aRenderProxy) const noexcept;

		void OnUpload(CommandContext& aCommandContext) noexcept;
	private:
		std::unordered_map<UUID, MeshRenderProxy> m_RenderData;
		std::unordered_map<UUID, uint32> m_IDToSlotMap;

		std::vector<ShaderInterop::MeshData> m_MeshCache;

		SceneBuffer m_MeshDataBuffer;

		CallbackID m_OnUploadCallbackID = INVALID_CALLBACK_ID;
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}
