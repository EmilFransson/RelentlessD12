#pragma once
namespace Relentless
{
	class ShaderLibrary;
	class RenderPass;
	class FrameBuffer;
	struct GraphicsCommandList;

	class MasterRenderer
	{
	public:
		static void Initialize() noexcept;
		static [[nodiscard]] ShaderLibrary& GetShaderLibrary() noexcept;
		static void OnShutDown() noexcept;
		static void Begin() noexcept;
		static void End() noexcept;
		static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> BeginRenderPass(const std::shared_ptr<RenderPass>& pRenderPass) noexcept;
		static void EndRenderPass(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept;
		static void PrepareBackBuffer() noexcept;
		[[nodiscard]] static const std::shared_ptr<FrameBuffer> GetFrameBuffer() noexcept;
		static void Resize(const uint32_t width, const uint32_t height) noexcept;
	};
}
