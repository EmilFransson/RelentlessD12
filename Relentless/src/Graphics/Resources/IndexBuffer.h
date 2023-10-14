#pragma once
#include "IResource.h"
namespace Relentless
{
	class IndexBuffer : public IResource
	{
	public:
		struct Specification
		{
			uint32_t NrOfIndices;
			uint32_t TotalSizeInBytes;
			uint32_t Stride;
			void* pBuffer;
			std::string Name;
		};
		IndexBuffer(const Specification* specification) noexcept;
		virtual ~IndexBuffer() noexcept override final = default;
		[[nodiscard]] const uint32_t GetNrOfIndices() const noexcept { return m_Specification.NrOfIndices; }
		[[nodiscard]] Specification& GetSpecification() noexcept { return m_Specification; }
	private:
		Specification m_Specification;
	};
}