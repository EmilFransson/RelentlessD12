#pragma once
#include "Subsystem/ISubsystem.h"

#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RenderProxy/PrimitiveRenderProxy.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/Texture.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	class MaterialRenderSubsystem;
	class MeshRenderSubsystem;

	class RLS_API PrimitiveRenderSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD uint32 GetNumInstances() const noexcept;
		NO_DISCARD const Buffer* GetRenderData() const;
		NO_DISCARD const std::vector<ShaderInterop::InstanceData>& GetInstanceCache() const;
		NO_DISCARD const PrimitiveRenderProxy& GetProxy(uint32 aEntityID) const;

		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		void OnUnload(ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		void Patch(std::vector<PrimitiveRenderProxy> someRenderProxyUpdates) noexcept;

		void Remove(std::vector<uint32> someIDs) noexcept;
	private:
		void BuildInstanceData(ShaderInterop::InstanceData& outInstanceData, const PrimitiveRenderProxy& aRenderProxy) const noexcept;

		void OnUpload(CommandContext& aCommandContext) noexcept;
	private:
		std::unordered_map<uint32, PrimitiveRenderProxy> m_RenderData;
		std::vector<ShaderInterop::InstanceData> m_InstanceCache;

		SceneBuffer m_InstanceDataBuffer;

		CallbackID m_OnUploadCallbackID = INVALID_CALLBACK_ID;

		MaterialRenderSubsystem* m_pMaterialRenderSubsystem = nullptr;
		MeshRenderSubsystem* m_pMeshRenderSubsystem = nullptr;
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}
