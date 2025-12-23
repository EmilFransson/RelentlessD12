#include "Mesh.h"
#include "Graphics/RHI/Buffer.h"

namespace Relentless
{
	Mesh::Mesh(Ref<Buffer> pVertexBuffer, Ref<Buffer> pIndexBuffer, const std::string& name) noexcept
		: m_pVertexBuffer{ pVertexBuffer }, m_pIndexBuffer{ pIndexBuffer }
	{
		SetName(name);
	}

	void Mesh::SetOffsetTransform(const Matrix& transform) noexcept
	{
		m_OffsetTransform = transform;
	}

	const Matrix& Mesh::GetOffsetTransform() const noexcept
	{
		return m_OffsetTransform;
	}

	const Relentless::AssetHandle& Mesh::GetDefaultMaterialHandle() noexcept
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

	void Mesh::SetDefaultMaterial(const AssetHandle& handle) noexcept
	{
		m_DefaultMaterialHandle = handle;
	}

}