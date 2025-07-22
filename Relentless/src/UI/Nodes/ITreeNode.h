#pragma once

#include "Callback/Callback.h"

namespace Relentless
{
	class IBaseWidget;

	class ITreeNode : public RefCounted<ITreeNode>
	{
	public:
		ITreeNode() noexcept = default;
		virtual ~ITreeNode() noexcept = default;

		NO_DISCARD Ref<IBaseWidget> CreateWidget() noexcept;

		void Filter(const String& filter) noexcept;

		NO_DISCARD const std::vector<Ref<ITreeNode>>& GetChildren() const noexcept;
		NO_DISCARD const std::vector<String>& GetFilterTags() const noexcept;
		NO_DISCARD const String& GetLabel() const noexcept;

		void SetFilterTags(const Span<String>& tags) noexcept;
		
		template<typename InstanceType>
		ITreeNode* SetWidgetCallback(InstanceType* instance, void(InstanceType::* method)()) noexcept
		{
			m_OnCreateWidgetCallback = [instance, method]() { return (instance->*method)(this); };
			return this;
		}

		template<typename T>
		ITreeNode* SetWidgetCallback(T&& callback) noexcept
		{
			m_OnCreateWidgetCallback = Callback<Ref<IBaseWidget>(ITreeNode*)>(std::forward<T>(callback));
			return this;
		}

		virtual bool MatchesFilterTag(const String& filter) const noexcept = 0;
	protected:
		std::vector<String> m_FilterTags;
		std::vector<Ref<ITreeNode>> m_Children;

		String m_Label;

		Callback<Ref<IBaseWidget>(ITreeNode* pOwningNode)> m_OnCreateWidgetCallback;

		bool m_MatchesFilter = true;
	};
}