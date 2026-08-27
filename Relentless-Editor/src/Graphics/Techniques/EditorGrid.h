#pragma once
#include <Relentless.h>

namespace Relentless
{
	class EditorGrid
	{
	public:
		static constexpr Color DEFAULT_MINOR_COLOR = Color(0.3f, 0.29f, 0.29f, 1.0f);
		static constexpr Color DEFAULT_MAJOR_COLOR = Color(0.06f, 0.06f, 0.06f, 1.0f);

		EditorGrid(GraphicsDevice* aGraphicsDevice) noexcept;
		void Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept;

		NO_DISCARD float GetDistanceFade() const noexcept;
		NO_DISCARD float GetHeightFade() const noexcept;
		NO_DISCARD float GetHeightOffset() const noexcept;
		NO_DISCARD const Color& GetMajorColor() const noexcept;
		NO_DISCARD const Color& GetMinorColor() const noexcept;
		NO_DISCARD float GetMaxOpacity() const noexcept;
		NO_DISCARD const Vector3& GetRotationRadians() const noexcept;
		NO_DISCARD float GetSpacing() const noexcept;

		void SetDistanceFade(float aFade) noexcept;
		void SetHeightFade(float aFade) noexcept;
		void SetHeightOffset(float aHeightOffset) noexcept;
		void SetMajorColor(const Color& aColor) noexcept;
		void SetMinorColor(const Color& aColor) noexcept;
		void SetMaxOpacity(float aMaxOpacity) noexcept;
		void SetRotationRadians(const Vector3& aRotationRadians) noexcept;
		void SetSpacing(float aSpacing) noexcept;
	private:
		void BuildInstanceBuffer();
	private:
		GraphicsDevice* m_pDevice = nullptr;
		Ref<Buffer> m_pInstancesStructuredBuffer = nullptr;

		Color m_ColorMajor = DEFAULT_MAJOR_COLOR;
		Color m_ColorMinor = DEFAULT_MINOR_COLOR;
		Vector3 m_RotationRadians = Vector3::Zero;
		float m_HeightOffset = 0.0f;
		float m_Spacing = 1.0f;
		float m_DistanceFade = 200.0f;
		float m_HeightFade = 150.0f;
		float m_MaxOpacity = 1.0f;
		bool m_Dirty = true;
	};
}