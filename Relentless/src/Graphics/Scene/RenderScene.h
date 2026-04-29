#pragma once
#include "Graphics/Renderer/RenderTypes.h"

#include "Subsystem/ISystemManager.h"

namespace Relentless
{
	class Renderer;

	class RenderScene : public ISystemManager
	{
	public:
		RenderScene(const UUID& aUUID, Renderer* aRenderer) noexcept;

		NO_DISCARD Span<const Batch> GetBatches() const noexcept;
		NO_DISCARD Renderer* GetRenderer() const noexcept;
		NO_DISCARD const UUID& GetUUID() const noexcept;

		void OnRenderBegin(const ViewRenderDesc& aViewRenderDesc) noexcept;

		void SortBatches(const ViewRenderDesc& aViewRenderDesc) noexcept;
	private:
		std::vector<Batch> m_Batches;
		UUID m_UUID;
		Renderer* m_pRenderer = nullptr;
	};
}