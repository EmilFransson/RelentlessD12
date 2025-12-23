#pragma once
#include "IWidget.h"

namespace Relentless
{
	class Thumbnail : public IStylableWidget<Thumbnail>
	{
	public:
		inline static constexpr Color DefaultBackgroundColor = Color(56.0f, 56.0f, 56.0f, 255.0f);

		Thumbnail(const Vector2& aSize) noexcept;
		virtual ~Thumbnail() noexcept override = default;

		NO_DISCARD float CalcDesiredWidth() const noexcept override;
		
		template<typename InstanceType>
		Thumbnail* ID(InstanceType* aInstance, uint64(InstanceType::*aMethod)() const) noexcept;

		template<typename InstanceType>
		Thumbnail* OnClicked(InstanceType* instance, void(InstanceType::* method)(Thumbnail*, const PointerInfo&)) noexcept;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		virtual Thumbnail* SetBackgroundColor(const Color& aColor) noexcept override;
	private:
		virtual void OnRender() noexcept override;
	private:
		Vector2 m_Size = Vector2::Zero;
		Callback<uint64()> m_ImageIDCallback;
		Callback<void(Thumbnail*, const PointerInfo&)> m_OnClickedCallback;

		Color m_BackgroundColor = DefaultBackgroundColor;
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