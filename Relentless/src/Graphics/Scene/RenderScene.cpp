#include "RenderScene.h"

#include "Graphics/Renderer/Renderer.h"

#include "Subsystem/CoreTypes/LightRenderSubsystem.h"
#include "Subsystem/CoreTypes/MaterialRenderSubsystem.h"
#include "Subsystem/CoreTypes/MeshRenderSubsystem.h"
#include "Subsystem/CoreTypes/PostProcessRenderSubsystem.h"
#include "Subsystem/CoreTypes/PrimitiveRenderSubsystem.h"
#include "Subsystem/CoreTypes/SkyBoxRenderSubsystem.h"
#include "Subsystem/CoreTypes/SkyLightRenderSubsystem.h"
#include "Subsystem/CoreTypes/SelectionRenderSubsystem.h"

namespace Relentless
{
	RenderScene::RenderScene(const UUID& aUUID, Renderer* aRenderer) noexcept
		:m_UUID{aUUID},
		 m_pRenderer{aRenderer}
	{
		//Default subsystem initialization (preserve order!):
		GetSubsystem<MaterialRenderSubsystem>();
		GetSubsystem<MeshRenderSubsystem>();
		GetSubsystem<PrimitiveRenderSubsystem>();
		GetSubsystem<LightRenderSubsystem>();
		GetSubsystem<SkyBoxRenderSubsystem>();
		GetSubsystem<SkyLightRenderSubsystem>();
		GetSubsystem<SelectionRenderSubsystem>();
		GetSubsystem<PostProcessRenderSubsystem>();
	}

	Span<const Batch> RenderScene::GetBatches() const noexcept
	{
		return m_Batches;
	}

	Renderer* RenderScene::GetRenderer() const noexcept
	{
		return m_pRenderer;
	}

	const UUID& RenderScene::GetUUID() const noexcept
	{
		return m_UUID;
	}

	void RenderScene::OnRenderBegin(const ViewRenderDesc& aViewRenderDesc) noexcept
	{
		//Build batches utilizing the info from all subsystems (already pre-run for this render frame!)
		PrimitiveRenderSubsystem* pPrimitiveRenderSubsystem = GetSubsystem<PrimitiveRenderSubsystem>();
		MeshRenderSubsystem* pMeshRenderSubsystem = GetSubsystem<MeshRenderSubsystem>();
		MaterialRenderSubsystem* pMaterialRenderSubsystem = GetSubsystem<MaterialRenderSubsystem>();

		m_Batches.clear();
		m_Batches.reserve(pPrimitiveRenderSubsystem->GetNumInstances());

		auto&& GetBlendMode = [](EBlendMode aRenderMode) -> Batch::Blending
			{
				switch (aRenderMode)
				{
				case EBlendMode::Opaque:		return Batch::Blending::Opaque;
				case EBlendMode::AlphaMask:		return Batch::Blending::AlphaMask;
				case EBlendMode::AlphaBlend:	return Batch::Blending::AlphaBlend;
				default:
					RLS_ASSERT(false, "Unreachable.");
					return Batch::Blending::Opaque;
				}
			};

		//Instances sorted by mesh and blend mode, which is reflected in uploaded GPU buffer.
		const std::vector<ShaderInterop::InstanceData>& instances = pPrimitiveRenderSubsystem->GetInstanceCache();

		size_t batchIndex = 0u;

		while (batchIndex < instances.size())
		{
			const ShaderInterop::InstanceData& baseInstanceData = instances[batchIndex];
			const PrimitiveRenderProxy& primitiveProxy = pPrimitiveRenderSubsystem->GetProxy(baseInstanceData.EntityID);
			const MaterialRenderProxy& baseMaterialProxy = pMaterialRenderSubsystem->GetProxy(primitiveProxy.MaterialUUID);
			const MeshRenderProxy& meshProxy = pMeshRenderSubsystem->GetProxy(primitiveProxy.MeshUUID);

			Batch& batch = m_Batches.emplace_back();
			batch.BaseInstanceOffset = static_cast<uint32>(batchIndex);
			batch.NumIndices = meshProxy.IndexBuffer->GetNrOfElements();
			batch.BlendMode = GetBlendMode(baseMaterialProxy.BlendMode);
			batch.Location = primitiveProxy.LocalToWorld.Translation(); //Purely relevant here for transparent meshes which are rendered one by one. (1 per batch)
			batch.IsTwoSided = baseMaterialProxy.IsTwoSided;

			while (batchIndex < instances.size())
			{
				++batch.InstanceCount;
				++batchIndex;

				if (batchIndex >= instances.size())
					break;

				const ShaderInterop::InstanceData& instanceData = instances[batchIndex];
				const PrimitiveRenderProxy& primitiveProxy = pPrimitiveRenderSubsystem->GetProxy(instanceData.EntityID);
				const MaterialRenderProxy& materialProxy = pMaterialRenderSubsystem->GetProxy(primitiveProxy.MaterialUUID);

				if (instanceData.MeshDataIndex != baseInstanceData.MeshDataIndex)
					break;

				if (materialProxy.BlendMode != baseMaterialProxy.BlendMode || materialProxy.BlendMode == EBlendMode::AlphaBlend)
					break;

				if (materialProxy.IsTwoSided != baseMaterialProxy.IsTwoSided)
					break;

				if (materialProxy.IsTwoSided && materialProxy.BlendMode == EBlendMode::AlphaMask)
					break;
			}
		}

		SortBatches(aViewRenderDesc);
	}

	void RenderScene::SortBatches(const ViewRenderDesc& aViewRenderDesc) noexcept
	{
		auto&& CompareSort = [&aViewRenderDesc](const Batch& a, const Batch& b)
			{
				if (a.BlendMode != b.BlendMode)
					return (int)a.BlendMode < (int)b.BlendMode;

				if (a.IsTwoSided != b.IsTwoSided)
					return (int)a.IsTwoSided < (int)b.IsTwoSided;

				const float aDist = Vector3::DistanceSquared(a.Location, aViewRenderDesc.ViewTransform.Location);
				const float bDist = Vector3::DistanceSquared(b.Location, aViewRenderDesc.ViewTransform.Location);

				if (a.BlendMode == Batch::Blending::AlphaBlend || a.BlendMode == Batch::Blending::AlphaMask)
					return aDist > bDist;

				return aDist < bDist;
			};
		std::sort(m_Batches.begin(), m_Batches.end(), CompareSort);
	}
}