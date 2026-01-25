#pragma once
#include <Relentless.h>
#include "IWidget.h"

#include "UI/Meta/ThumbnailInfo.h"

namespace Relentless
{
	class Thumbnail : public IStylableWidget<Thumbnail>
	{
	public:
		inline static constexpr Color DefaultBackgroundColor = Color(56.0f, 56.0f, 56.0f, 255.0f);

		Thumbnail() noexcept = default;
		Thumbnail(const Vector2& aSize, const Ref<ThumbnailInfo>& aInfo) noexcept;
		virtual ~Thumbnail() noexcept override = default;

		NO_DISCARD float CalcDesiredWidth() const noexcept override;
		
		NO_DISCARD const Ref<Texture>& GetResource() const noexcept;

		template<typename InstanceType>
		Thumbnail* ID(InstanceType* aInstance, uint64(InstanceType::*aMethod)() const) noexcept;

		NO_DISCARD bool Load(LoadArchive& aArchive) noexcept;

		template<typename InstanceType>
		Thumbnail* OnClicked(InstanceType* instance, void(InstanceType::*method)(Thumbnail*, const PointerInfo&)) noexcept;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		virtual Thumbnail* SetBackgroundColor(const Color& aColor) noexcept override;
		void SetInfo(const Ref<ThumbnailInfo>& aInfo) noexcept;
		void SetResource(const Ref<Texture>& aTexture);
		void SetSize(const Vector2& aSize) noexcept;
	private:
		virtual void OnRender() noexcept override;
	private:
		Ref<ThumbnailInfo> m_pThumbnailInfo = nullptr;
		Ref<ThumbnailRenderData> m_pRenderData = nullptr;
		Vector2 m_Size = Vector2::Zero;
		Callback<uint64()> m_ImageIDCallback;
		Callback<void(Thumbnail*, const PointerInfo&)> m_OnClickedCallback;

		Color m_BackgroundColor = DefaultBackgroundColor;

		Ref<Texture> m_pResource = nullptr;
	};

	template<typename InstanceType>
	Thumbnail* Thumbnail::ID(InstanceType* aInstance, uint64(InstanceType::*aMethod)() const) noexcept
	{
		m_ImageIDCallback = [aInstance, aMethod]() { return (aInstance->*aMethod)(); };
		return this;
	}

	template<typename InstanceType>
	Thumbnail* Thumbnail::OnClicked(InstanceType* aInstance, void(InstanceType::*aMethod)(Thumbnail*, const PointerInfo&)) noexcept
	{
		m_OnClickedCallback = [aInstance, aMethod](Thumbnail* pThumbnail, const PointerInfo& pointerInfo) { return (aInstance->*aMethod)(pThumbnail, pointerInfo); };
		return this;
	}
}