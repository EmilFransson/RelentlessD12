#pragma once
#include "../Resources/Texture.h"
namespace Relentless
{
	class Renderer3D
	{
	public:
		static void Initialize() noexcept;
		static void Begin() noexcept;
		static void Submit() noexcept;
		static void End() noexcept;
		static void ExecuteCommands() noexcept;
		static void WaitAndSync() noexcept;
		static void WaitForGPU() noexcept;
		static void OnShutDown() noexcept;
		static [[nodiscard]] const std::shared_ptr<RenderTextureMSAA>& GetUITexture() noexcept;
		static void OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept;
	private:
		Renderer3D() noexcept = delete;
		~Renderer3D() noexcept = delete;
	};
}