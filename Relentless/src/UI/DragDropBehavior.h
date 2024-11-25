#pragma once
#include "Core/Any.h"
#include "Callback/Broadcaster.h"

namespace Relentless
{
	class DragDropBehavior
	{
	public:
		explicit DragDropBehavior(const char* id) noexcept;

		[[nodiscard]] bool BeginDragDropSource() noexcept;
		void EndDragDropSource() noexcept;
		[[nodiscard]] bool BeginDragDropTarget() noexcept;
		void EndDragDropTarget() noexcept;

		void SetPayload(const Any& payload) noexcept;
		void DropOver(const Any& target, std::string_view dropContext) noexcept;
		[[nodiscard]] bool IsActive() const noexcept;
		[[nodiscard]] bool OnHoverDragDropTarget(const Any& target, std::string_view dropContext) noexcept;
		[[nodiscard]] bool HasPayload() const noexcept;

		void TryPayloadDelivery(const Any& target, std::string_view dropContext) noexcept;

		Broadcaster<bool(const Any& payload, const Any& target, std::string_view dropContext)> OnDragOver;
		Broadcaster<void(const Any& payload, const Any& target, std::string_view dropContext)> OnDrop;
	private:
		bool m_CurrentTargetIsValid = false;
		Any m_Payload;
		const char* m_ID = nullptr;
		bool m_IsDragging = false;
	};
}