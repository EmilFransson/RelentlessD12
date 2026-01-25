#pragma once
#include <Relentless.h>
#include "IWidget.h"

namespace Relentless
{
	class EditableTextBox : public IStylableWidget<EditableTextBox>
	{
	public:
		EditableTextBox(const Vector2& aSize = Vector2::Zero, std::string_view aHintText = "") noexcept;
		virtual ~EditableTextBox() noexcept override = default;

		NO_DISCARD virtual float CalcDesiredWidth() const noexcept override;

		virtual void OnRender() noexcept override;
		void SetText(const String& aText) noexcept;
		
		template<typename Func>
		EditableTextBox* OnTextChanged(Func&& callback) noexcept
		{
			m_OnTextChanged = std::move(callback);
			return this;
		}

		template<typename InstanceType>
		EditableTextBox* OnTextChanged(InstanceType* instance, void(InstanceType::* method)(const char*)) noexcept
		{
			m_OnTextChanged = [instance, method](const char* pText) { return (instance->*method)(pText); };
			return this;
		}

		template<typename Func>
		EditableTextBox* OnTextCommitted(Func&& callback) noexcept
		{
			m_OnTextCommitted = callback;
			return this;
		}

		template<typename InstanceType>
		EditableTextBox* OnTextCommitted(InstanceType* instance, void(InstanceType::* method)(const char*, ETextCommitType)) noexcept
		{
			m_OnTextCommitted = [instance, method](const char* pText, ETextCommitType commitType) { return (instance->*method)(pText, commitType); };
			return this;
		}

		NO_DISCARD virtual Vector2 ReportSize() const noexcept override;

	private:
		Callback<void(const char*)> m_OnTextChanged;
		Callback<void(const char* pText, ETextCommitType commitType)> m_OnTextCommitted;

		char m_InputBuffer[128];
		String m_HintText{};
		Vector2 m_Size = Vector2::Zero;
		bool m_IsActive = false;
	};
}