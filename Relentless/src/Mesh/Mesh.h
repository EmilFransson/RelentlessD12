#pragma once
#include "Graphics/Resources/IndexBuffer.h"
#include "Graphics/Resources/VertexBuffer.h"

namespace Relentless
{
	class Mesh
	{
	public:
		explicit Mesh(const std::string& name, const VertexBuffer::Specification& vbSpec, const IndexBuffer::Specification& ibSpec) noexcept;
		Mesh();
		void SetName(const std::string& name) noexcept;
		void SetVertexBuffer(std::unique_ptr<VertexBuffer>&& pVertexBuffer) noexcept;
		void SetIndexBuffer(std::unique_ptr<IndexBuffer>&& pIndexBuffer) noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] const std::unique_ptr<VertexBuffer>& GetVertexBuffer() const noexcept { return m_pVertexBuffer; }
		[[nodiscard]] const std::unique_ptr<IndexBuffer>& GetIndexBuffer() const noexcept { return m_pIndexBuffer; }
	private:
		std::string m_Name;
		std::unique_ptr<VertexBuffer> m_pVertexBuffer;
		std::unique_ptr<IndexBuffer> m_pIndexBuffer;
	};
}