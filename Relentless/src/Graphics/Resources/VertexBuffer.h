#pragma once
#include "IResource.h"
namespace Relentless
{
	class VertexBuffer : public IResource
	{
	public:
		struct Specification
		{
			uint32_t NrOfVertices;
			uint32_t TotalSizeInBytes;
			uint32_t Stride;
			void* pBuffer;
			std::string Name;
		};
		VertexBuffer(const Specification* specification) noexcept;
		virtual ~VertexBuffer() noexcept override final = default;
		[[nodiscard]] const uint32_t GetNrOfVertices() const noexcept { return m_NrOfVertices; }
	private:
		uint32_t m_NrOfVertices;
	};
}