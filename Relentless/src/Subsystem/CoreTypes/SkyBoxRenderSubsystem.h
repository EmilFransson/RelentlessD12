#pragma once
#include "Subsystem/ISubsystem.h"

#include "Graphics/RenderProxy/SkyBoxRenderProxy.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/Texture.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	enum class ESkyBoxRenderProxyUpdateType : uint8 { Patch = 0u, Delete };

	struct PendingSkyBoxRenderProxyUpdate
	{
		SkyBoxRenderProxy RenderProxy;
		ESkyBoxRenderProxyUpdateType UpdateType;

		NO_DISCARD static PendingSkyBoxRenderProxyUpdate MakeDeletion(uint32 aID) noexcept
		{
			PendingSkyBoxRenderProxyUpdate update{};
			update.UpdateType = ESkyBoxRenderProxyUpdateType::Delete;
			update.RenderProxy.ID = aID;
			return update;
		}
	};

	class CommandContext;
	class GraphicsDevice;

	class RLS_API SkyBoxRenderSubsystem : public ISubsystem
	{
	public:
		inline static const uint32 INVALID_ID = 0xFFFFFFFF;

		NO_DISCARD const Buffer* GetRenderData() const;

		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		void Patch(std::vector<SkyBoxRenderProxy> someRenderProxyUpdates) noexcept;

		void Remove(std::vector<uint32> someIDs) noexcept;

		NO_DISCARD bool ShouldBlendEnvironments() const noexcept;
	private:
		void ApplyPatch(SkyBoxRenderProxy aRenderProxy) noexcept;

		void BuildSkyBoxData(ShaderInterop::SkyboxData& outSkyBoxData) const noexcept;

		void FlushPendingProxyUpdates() noexcept;

		void OnRenderFrameBegin();
		void OnUpload(CommandContext& aCommandContext) noexcept;

		void RemoveByID(uint32 aID) noexcept;

		void UpdateActiveID(const SkyBoxRenderProxy& aRenderProxy) noexcept;
	private:
		std::unordered_map<uint32, SkyBoxRenderProxy> m_RenderData;
		std::vector<PendingSkyBoxRenderProxyUpdate> m_PendingRenderProxyUpdates;

		uint32 m_ActiveID = INVALID_ID;

		GraphicsDevice* m_pGraphicsDevice = nullptr;
		Ref<Buffer> m_pSkyBoxDataBuffer = nullptr;
	};
}