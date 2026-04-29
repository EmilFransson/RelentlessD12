#include "Mesh.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Material.h"

#include "Core/Application.h"

#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"

namespace Relentless
{
	Mesh::Mesh(Ref<Buffer> aVertexBuffer, Ref<Buffer> aIndexBuffer, const String& aName) noexcept
		: m_pVertexBuffer{ aVertexBuffer }, m_pIndexBuffer{ aIndexBuffer }
	{
		SetName(aName);
	}

	Mesh::Mesh(const UUID& aUUID) noexcept
		: AssetBase<Mesh>(aUUID)
	{
	}

	Mesh::Mesh() noexcept
		: AssetBase<Mesh>(CreateUUID())
	{
	}

	Mesh::~Mesh() noexcept = default;

	Ref<Material> Mesh::GetDefaultMaterial() noexcept
	{
		RLS_ASSERT(m_DefaultMaterialHandle.IsValid(), "[Mesh::GetDefaultMaterial]: Default material handle is invalid.");
		return AssetManager::Get<Material>(m_DefaultMaterialHandle);
	}

	const Matrix& Mesh::GetOffsetTransform() const noexcept
	{
		return m_OffsetTransform;
	}

	const AssetHandle& Mesh::GetDefaultMaterialHandle() noexcept
	{
		return m_DefaultMaterialHandle;
	}

	Buffer* Mesh::GetVertexBuffer() const noexcept
	{
		RLS_ASSERT(m_pVertexBuffer, "[Mesh::GetVertexBuffer] Vertex Buffer Is Invalid.");
		return m_pVertexBuffer;
	}

	Buffer* Mesh::GetIndexBuffer() const noexcept
	{
		RLS_ASSERT(m_pIndexBuffer, "[Mesh::GetIndexBuffer] Index Buffer Is Invalid.");
		return m_pIndexBuffer;
	}

	void Mesh::SetDefaultMaterial(const AssetHandle& aMaterialHandle) noexcept
	{
		RLS_ASSERT(aMaterialHandle.Type == Material::StaticType(), "[Mesh::SetDefaultMaterial]: Asset handle is not of material type.");

		if (m_DefaultMaterialHandle == aMaterialHandle)
			return;

		m_DefaultMaterialHandle = aMaterialHandle;
		NOTIFY_PROPERTY_CHANGED(m_DefaultMaterialHandle);
	}

	void Mesh::SetOffsetTransform(const Matrix& aOffsetTransform) noexcept
	{
		if (m_OffsetTransform == aOffsetTransform)
			return;

		m_OffsetTransform = aOffsetTransform;
		NOTIFY_PROPERTY_CHANGED(m_OffsetTransform);
	}

	bool Mesh::SerializeCore(IArchive& aArchive) noexcept
	{
		if (aArchive.IsSaving())
		{
			const BufferDesc& vbDesc = m_pVertexBuffer->GetDesc();
			const BufferDesc& ibDesc = m_pIndexBuffer->GetDesc();

			return aArchive.Process(vbDesc)
				&& aArchive.Process(ibDesc)
				&& aArchive.Process(m_OffsetTransform)
				&& aArchive.Process(m_Bounds)
				&& aArchive.Process(m_DefaultMaterialHandle)
				&& aArchive.IsValid();
		}
		else
		{
			BufferDesc vbDesc{};
			BufferDesc ibDesc{};

			return aArchive.Process(vbDesc)
				&& aArchive.Process(ibDesc)
				&& aArchive.Process(m_OffsetTransform)
				&& aArchive.Process(m_Bounds)
				&& aArchive.Process(m_DefaultMaterialHandle)
				&& aArchive.IsValid();
		}
	}

	bool Mesh::SerializeBulk(IArchive& aArchive) noexcept
	{
		GraphicsDevice* pGraphicsDevice = Application::Get().GetGraphicsDevice();

		if (aArchive.IsSaving())
		{
			const BufferDesc& vertexBufferDesc = m_pVertexBuffer->GetDesc();
			const BufferDesc& indexBufferDesc = m_pIndexBuffer->GetDesc();

			aArchive.Process(vertexBufferDesc);
			aArchive.Process(indexBufferDesc);

			const uint64 vertexBufferSize = vertexBufferDesc.Size;
			const uint64 indexBufferSize = indexBufferDesc.Size;
			constexpr uint64 alignment = 4u;

			const uint64 indexBufferOffset = Math::AlignUp(vertexBufferSize, alignment);
			const uint64 blobSize = indexBufferOffset + indexBufferSize;

			Ref<Buffer> pReadbackBuffer = pGraphicsDevice->CreateBuffer(BufferDesc::CreateReadback(blobSize), "Mesh Readback buffer");
			
			pGraphicsDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY)->InsertWait(pGraphicsDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
			CommandContext* pCommandContext = pGraphicsDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COPY);
			
			const D3D12_RESOURCE_STATES currentVBResourceState = m_pVertexBuffer->GetResourceState();
			const D3D12_RESOURCE_STATES currentIBResourceState = m_pIndexBuffer->GetResourceState();

			pCommandContext->InsertResourceBarrier(m_pVertexBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
			pCommandContext->InsertResourceBarrier(m_pIndexBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

			pCommandContext->CopyBuffer(m_pVertexBuffer, pReadbackBuffer, vertexBufferSize, 0u, 0u);
			pCommandContext->CopyBuffer(m_pIndexBuffer, pReadbackBuffer, indexBufferSize, 0u, indexBufferOffset);

			pCommandContext->InsertResourceBarrier(m_pVertexBuffer, currentVBResourceState);
			pCommandContext->InsertResourceBarrier(m_pIndexBuffer, currentIBResourceState);

			pCommandContext->Execute().Wait();

			const D3D12_RANGE readRange{ 0, static_cast<SIZE_T>(blobSize) };
			pReadbackBuffer->Map(0u, &readRange);

			auto* base = (std::byte*)pReadbackBuffer->GetMappedData();
			aArchive.ProcessRaw(base + 0, vertexBufferSize);
			aArchive.ProcessRaw(base + indexBufferOffset, indexBufferSize);

			const D3D12_RANGE writtenRange{ 0, 0 };
			pReadbackBuffer->Unmap(0u, &writtenRange);

			return true;
		}
		else //Loading
		{
			BufferDesc vertexBufferDesc{};
			BufferDesc indexBufferDesc{};

			if (!aArchive.Process(vertexBufferDesc))
				return false;

			if (!aArchive.Process(indexBufferDesc))
				return false;

			const uint64 vertexBufferSize = vertexBufferDesc.Size;
			const uint64 indexBufferSize = indexBufferDesc.Size;
			constexpr uint64 alignment = 4u;

			const uint64 indexBufferOffset = Math::AlignUp(vertexBufferSize, alignment);
			const uint64 blobSize = indexBufferOffset + indexBufferSize;

			std::vector<std::byte> blob;
			blob.resize((size_t)blobSize);

			if (!aArchive.ProcessRaw(blob.data(), blobSize))
				return false;

			const void* vbInit = blob.data() + 0;
			const void* ibInit = blob.data() + indexBufferOffset;

			m_pVertexBuffer = pGraphicsDevice->CreateBuffer(vertexBufferDesc, "Mesh VB", vbInit);
			m_pIndexBuffer = pGraphicsDevice->CreateBuffer(indexBufferDesc, "Mesh IB", ibInit);

			return (m_pVertexBuffer != nullptr) && (m_pIndexBuffer != nullptr);
		}
	}
}