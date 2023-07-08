#pragma once
#include "Properties.h"
#include "../Resources/Texture.h"
#include "../Resources/DepthStencil.h"
namespace Relentless
{
	class FrameBuffer;

	enum class OperatorOnLoad : uint8_t
	{
		LoadOnly = 0,
		Clear
	};

	enum class DepthComparisonFunction : uint8_t 
	{
		LESS = 0,
		LESS_EQUAL,
		GREATER,
		GREATER_EQUAL,
		EQUAL,
		NOT_EQUAL,
		NEVER,
		ALWAYS
	};

	struct ColorAttachment
	{
		std::shared_ptr<RenderTexture> Output{ nullptr };
		DirectX::XMFLOAT4 ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		TextureFormat Format{ TextureFormat::None };
		OperatorOnLoad OperatorOnLoad{ OperatorOnLoad::Clear };
		bool Transfer{ false };
		bool IsSRGB{ true };
		bool Blend{ false };
		D3D12_RESOURCE_FLAGS Flags{ D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET };
		std::shared_ptr<FrameBuffer> pOutputDependency{ nullptr };
	};

	struct DepthAttachment
	{
		std::shared_ptr<DepthStencil> Output{ nullptr };
		TextureFormat Format { TextureFormat::None };
		OperatorOnLoad OperatorOnLoad{ OperatorOnLoad::Clear };
		std::shared_ptr<FrameBuffer> pOutputDependency{ nullptr };
	};

	struct Attachments
	{
		std::vector<ColorAttachment> ColorAttachments;
		DepthAttachment DepthAttachment;
	};

	struct FrameBufferSpecification
	{
		std::string DebugName = "?";
		uint32_t Width{800u};
		uint32_t Height{600u};
		uint8_t MSAASamples{1u};
		Attachments Attachments{};
		DepthComparisonFunction DepthComparisonFunction{ DepthComparisonFunction::LESS_EQUAL };
		bool ShouldResize{ true };
	};

	class FrameBuffer
	{
	public:
		explicit FrameBuffer(const FrameBufferSpecification& specification) noexcept;
		~FrameBuffer() noexcept = default;
		static [[nodiscard]] std::shared_ptr<FrameBuffer> Create(const FrameBufferSpecification& specification) noexcept;
		[[nodiscard]] const FrameBufferSpecification& GetSpecification() const noexcept { return m_Specification; }
		void Resize(uint32_t width, uint32_t height) noexcept;
		[[nodiscard]] std::shared_ptr<RenderTexture> GetOutput(const uint32_t outputIndex) const noexcept { return m_Specification.Attachments.ColorAttachments[outputIndex].Output; }
		[[nodiscard]] std::shared_ptr<DepthStencil> GetDepthOutput() const noexcept { return m_Specification.Attachments.DepthAttachment.Output; }
		void OnMSAAReconfiguration(uint8_t nrOfSamples) noexcept; //TODO: Have private, make pipeline friend.
		void SynchronizeDependencies() noexcept;
		void SetOutputDependency(std::shared_ptr<FrameBuffer> dependency, uint32_t outputSlot) noexcept;
		void SetDepthDependency(std::shared_ptr<FrameBuffer> dependency) noexcept;
	private:
		FrameBufferSpecification m_Specification;
	};
}