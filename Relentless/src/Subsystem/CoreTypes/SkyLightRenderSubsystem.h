#pragma once
#include "Subsystem/ISubsystem.h"

#include "Graphics/RenderProxy/SkyLightRenderProxy.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/Texture.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	enum class ESkyLightRenderProxyUpdateType : uint8 { Patch = 0u, Delete };

	struct PendingSkyLightRenderProxyUpdate
	{
		SkyLightRenderProxy RenderProxy;
		ESkyLightRenderProxyUpdateType UpdateType;

		NO_DISCARD static PendingSkyLightRenderProxyUpdate MakeDeletion(uint32 aID) noexcept
		{
			PendingSkyLightRenderProxyUpdate update{};
			update.UpdateType = ESkyLightRenderProxyUpdateType::Delete;
			update.RenderProxy.ID = aID;
			return update;
		}
	};

	struct SkyLightRenderDataEntry
	{
		Ref<Texture> IrradianceMap = nullptr;
		Ref<Texture> RadianceMap = nullptr;
		uint32 TargetMip = 0u;
		bool IrradianceDirty = false;
		bool RadianceDirty = false;
	};

	struct SkyLightRenderData
	{
		SkyLightRenderProxy RenderProxy;
		SkyLightRenderDataEntry PrimaryEntry;
		SkyLightRenderDataEntry SecondaryEntry;
	};

	class CommandContext;
	class GraphicsDevice;
	class IBLGenerationService;

	class RLS_API SkyLightRenderSubsystem : public ISubsystem
	{
	public:
		inline static const uint32 INVALID_SKYLIGHT_ID = 0xFFFFFFFF;

		NO_DISCARD const Buffer* GetRenderData() const;

		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		void OnUnload(ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		void Patch(std::vector<SkyLightRenderProxy> someRenderProxyUpdates) noexcept;

		void Remove(std::vector<uint32> someIDs) noexcept;

		void SetBRDFLutTexture(Ref<Texture> aBRDFLUT) noexcept;
	private:
		void ApplyPatch(SkyLightRenderProxy aRenderProxy) noexcept;
		
		void BuildSkyLightData(ShaderInterop::SkyLightData& outSkyLightData) const noexcept;
		
		void DispatchIBLRequests(SkyLightRenderData& aRenderData) noexcept;

		void EnsureIBLMapsAllocated(const SkyLightRenderProxy& aRenderProxy, SkyLightRenderData& outRenderData) noexcept;

		void FlushPendingProxyUpdates() noexcept;

		void OnRenderFrameBegin();
		void OnUpload(CommandContext& aCommandContext) noexcept;

		void RemoveByID(uint32 aID) noexcept;

		void UpdateActiveID(const SkyLightRenderProxy& aRenderProxy) noexcept;
	private:
		std::unordered_map<uint32, SkyLightRenderData> m_RenderData;
		std::vector<PendingSkyLightRenderProxyUpdate> m_PendingRenderProxyUpdates;

		uint32 m_ActiveSkyLightID = INVALID_SKYLIGHT_ID;
		CallbackID m_OnFrameBeginCallbackID = INVALID_CALLBACK_ID;
		CallbackID m_OnUploadCallbackID = INVALID_CALLBACK_ID;

		GraphicsDevice* m_pGraphicsDevice = nullptr;
		Ref<Buffer> m_pSkyLightDataBuffer = nullptr;
		Ref<Texture> m_pBRDFLut = nullptr;

		IBLGenerationService* m_pIBLGenerationService = nullptr;
	};
}