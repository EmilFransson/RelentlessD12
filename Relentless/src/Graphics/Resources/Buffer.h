#pragma once 
#include "IResource.h"
namespace Relentless
{
	class Buffer : public IResource
	{
	public:
		[[nodiscard]] constexpr const uint32_t GetSize() const noexcept { return m_SizeInBytes; }
	protected:
		explicit Buffer(const uint32_t Size, const std::string& name = "?") noexcept;
		Buffer() noexcept = default;
		virtual ~Buffer() noexcept override = default;
	private:
		uint32_t m_SizeInBytes;
	};

	class ReadBackBuffer : public Buffer 
	{
	public: 
		explicit ReadBackBuffer(const uint32_t sizeInBytes, const std::string& name = "?") noexcept;
		virtual ~ReadBackBuffer() noexcept final = default;
		[[nodiscard]] static std::shared_ptr<ReadBackBuffer> Create(const uint32_t sizeInBytes, const std::string& name = "?") noexcept;
	};
}