#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class EResolutionMode : uint8
	{
		Native,      //Render size == display size.
		Scale,       //Render size == display size * ScaleFactor.
		Fixed,       //Render size pinned; display size ignored except for aspect.
		FixedHeight  //Height pinned, width derived from display aspect. The retro case.
	};

	class ViewportSurface final
	{
	public:
		struct Desc
		{
			friend class ViewportSurface;
		public:
			ResourceFormat Format = ResourceFormat::RGB10A2_UNORM;
			TextureFlag Flags  = TextureFlag::ShaderResource | TextureFlag::UnorderedAccess;
			String DebugName = "Viewport Render Texture";

			NO_DISCARD static Desc Fixed(const Vector2u& aSize) noexcept;
			NO_DISCARD static Desc FixedHeight(uint32 aHeight) noexcept;

			NO_DISCARD static Desc Native() noexcept;

			NO_DISCARD static Desc Scaled(float aScale) noexcept;

			NO_DISCARD constexpr EResolutionMode GetMode() const noexcept { return m_Mode; }
		private:
			Vector2u m_FixedSize = Vector2u::Zero();
			uint32 m_FixedHeight = 0.0f;
			float m_Scale = 1.0f;
			EResolutionMode m_Mode = EResolutionMode::Native;
		};

		explicit ViewportSurface(const Desc& aDesc);

		ViewportSurface(const ViewportSurface&) = delete;
		ViewportSurface& operator=(const ViewportSurface&) = delete;

		void Flush() noexcept;

		NO_DISCARD float GetAspectRatio() const noexcept;
		NO_DISCARD Vector2u GetSize() const noexcept;
		NO_DISCARD Texture* GetTexture() const noexcept;

		void RequestResize(const Vector2u& aSize) noexcept;

		void SetResolutionMode(EResolutionMode aMode) noexcept;
	private:
		NO_DISCARD Vector2u ComputeRenderSize() noexcept;
	private:
		Desc m_Desc;
		Vector2u m_RequestedSize = Vector2u::Zero();
		Ref<Texture> m_pTexture = nullptr;
		bool m_Dirty = true;
	};
}