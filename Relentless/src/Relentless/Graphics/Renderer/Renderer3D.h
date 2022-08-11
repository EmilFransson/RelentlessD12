#pragma once
namespace Relentless
{
	class RenderTexture;
	class Triangle;
	class Renderer3D
	{
	public:
		static void Initialize() noexcept;
		static void Begin() noexcept;
		static void Submit(const std::shared_ptr<Triangle>& pTriangle) noexcept;
		static void End() noexcept;
		static void PrepareBackBuffer() noexcept;
		static void ExecuteCommands() noexcept;
		static void WaitAndSync() noexcept;
		static void WaitForGPU() noexcept;
		static void OnShutDown() noexcept;
		static [[nodiscard]] const std::shared_ptr<RenderTexture>& GetViewportTexture() noexcept;
		static void OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept;
		static void CreateTestRootSignature() noexcept;
		static void CreateTestPipelineState() noexcept;
	private:
		Renderer3D() noexcept = delete;
		~Renderer3D() noexcept = delete;
	};
}