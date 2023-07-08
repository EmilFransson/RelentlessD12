#pragma once
namespace Relentless
{
	class ShaderLibrary;
	class RenderPass;
	class FrameBuffer;

	class MasterRenderer
	{
	public:
		static void Initialize() noexcept;
		static [[nodiscard]] ShaderLibrary& GetShaderLibrary() noexcept;
		static void ExecuteCommands() noexcept;
		static void WaitAndSync() noexcept;
		static void WaitAndSyncAllFramesInFlight() noexcept;
		static void WaitForGPU() noexcept;
		static void OnShutDown() noexcept;
		static void ResetFrameCommandUnits(const uint32_t frameIndex) noexcept;
		static [[nodiscard]] uint32_t GetCurrentFrameIndex() noexcept;
		static void Begin() noexcept;
		static void End() noexcept;
		static void BeginRenderPass(const std::shared_ptr<RenderPass>& pRenderPass) noexcept;
		static void EndRenderPass() noexcept;
		static void PrepareBackBuffer() noexcept;
		[[nodiscard]] static const std::shared_ptr<FrameBuffer> GetFrameBuffer() noexcept;
		static void Resize(const uint32_t width, const uint32_t height) noexcept;
	};
}
