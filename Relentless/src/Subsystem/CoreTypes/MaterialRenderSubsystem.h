#pragma once
#include "Subsystem/ISubsystem.h"

#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RenderProxy/MaterialRenderProxy.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/Texture.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	class RLS_API MaterialRenderSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD uint32 GetNumMaterials() const noexcept;
		NO_DISCARD const Buffer* GetRenderData() const noexcept;
		NO_DISCARD uint32 GetSlotIndex(const UUID& aUUID) const noexcept;
		NO_DISCARD const MaterialRenderProxy& GetProxy(const UUID& aUUID) const noexcept;

		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		void OnUnload(ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		void Patch(std::vector<MaterialRenderProxy> someRenderProxyUpdates) noexcept;

		void Remove(std::vector<UUID> someIDs) noexcept;
	private:
		void BuildMaterialData(ShaderInterop::Material& outMaterialData, const MaterialRenderProxy& aRenderProxy) const noexcept;

		void OnUpload(CommandContext& aCommandContext) noexcept;
	private:
		std::unordered_map<UUID, MaterialRenderProxy> m_RenderData;
		std::unordered_map<UUID, uint32> m_IDToSlotMap;

		std::vector<ShaderInterop::Material> m_MaterialCache;

		SceneBuffer m_MaterialDataBuffer;

		CallbackID m_OnUploadCallbackID = INVALID_CALLBACK_ID;
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}
