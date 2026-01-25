#pragma once
namespace Relentless
{
	struct SceneState;

	class ISystem
	{
	public:
		ISystem() noexcept = default;
		virtual ~ISystem() noexcept = default;

		virtual void Execute(SceneState& aSceneState) noexcept = 0;

		NO_DISCARD float GetTickRate() const noexcept;

		void SetTickRate(const float aTickRate) noexcept;
	private:
		float m_TickRate = 0.0f;
	};
}