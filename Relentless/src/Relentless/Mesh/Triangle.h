#pragma once
#include "Vertex.h"
namespace Relentless
{
	class Triangle
	{
	public:
		Triangle() noexcept;
		virtual ~Triangle() noexcept = default;
		[[nodiscard]] constexpr const Microsoft::WRL::ComPtr<ID3D12Resource>& GetVertexBuffer() const noexcept { return m_pVertexBuffer; }
		[[nodiscard]] constexpr const Microsoft::WRL::ComPtr<ID3D12Resource>& GetIndexBuffer() const noexcept { return m_pIndexBuffer; }
		[[nodiscard]] constexpr const uint32_t GetNrOfVertices() const noexcept { return m_NrOfVertices; }
		[[nodiscard]] constexpr const uint32_t GetNrOfIndices() const noexcept{ return m_NrOfIndices; }
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pIndexBuffer;

		uint32_t m_NrOfVertices;
		uint32_t m_NrOfIndices;
	};
}