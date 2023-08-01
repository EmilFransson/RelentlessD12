#pragma once
#include <Relentless.h>
namespace Relentless
{
	enum class MSAA : uint8_t { OFF = 0, TWO, FOUR, EIGHT };
	class SceneRendererPanel
	{
	public:
		explicit SceneRendererPanel() noexcept = default;
		~SceneRendererPanel() noexcept = default;
		void OnImGuiRender(const bool show) noexcept;
		void SetActiveRenderer(std::shared_ptr<SceneRenderer> pSceneRenderer) noexcept;
		void OnPostRender() noexcept;
	private:
		template<typename LambdaFunction>
		void DrawColumnSection(const char* label, uint32_t columnWidth, const LambdaFunction&& invocable, bool border = true) noexcept;
	private:
		std::shared_ptr<SceneRenderer> m_pContext;
		MSAA m_MSAAOption;
		std::queue<std::function<void()>> m_DeferredFunctionCalls;
	};
}