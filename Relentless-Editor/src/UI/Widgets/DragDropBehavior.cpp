#include "DragDropBehavior.h"
#include "ImGui/ImGuiIncludes.h"

namespace Relentless
{
	DragDropBehavior::DragDropBehavior(const char* id) noexcept
		: m_ID(id)
	{
	}

	bool DragDropBehavior::BeginDragDropSource() noexcept
	{
		if (ImGui::BeginDragDropSource())
		{
			m_IsDragging = true;
			ImGui::SetDragDropPayload(m_ID, nullptr, 0, ImGuiCond_Once);
			return true;
		}
		else if (m_IsDragging)
		{
			if (!ImGui::IsMouseDragging(ImGuiMouseButton_::ImGuiMouseButton_Left) && !ImGui::IsDragDropActive())
			{
				m_IsDragging = false;
				m_Payload.Reset();
			}
			return false;
		}

		return false;
	}

	void DragDropBehavior::EndDragDropSource() noexcept
	{
		ImGui::EndDragDropSource();
	}

	bool DragDropBehavior::BeginDragDropTarget() noexcept
	{
		return ImGui::BeginDragDropTarget();
	}

	void DragDropBehavior::EndDragDropTarget() noexcept
	{
		ImGui::EndDragDropTarget();
	}

	void DragDropBehavior::SetPayload(const Any& payload) noexcept
	{
		m_Payload = payload;
	}

	void DragDropBehavior::DropOver(const Any& target, std::string_view dropContext) noexcept
	{
		OnDrop(m_Payload, target, dropContext);
		m_Payload.Reset();
	}

	bool DragDropBehavior::IsActive() const noexcept
	{
		return m_IsDragging;
	}

	bool DragDropBehavior::OnHoverDragDropTarget(const Any& target, std::string_view dropContext) noexcept
	{
		const std::vector<bool> results = OnDragOver(m_Payload, target, dropContext);
		return std::any_of(results.begin(), results.end(), [](bool state)
			{
				return state == true;
			});
	}

	bool DragDropBehavior::HasPayload() const noexcept
	{
		return m_Payload.HasValue();
	}

	void DragDropBehavior::TryPayloadDelivery(const Any& target, std::string_view dropContext) noexcept
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(m_ID, ImGuiDragDropFlags_::ImGuiDragDropFlags_AcceptBeforeDelivery);
		
		if (!payload->IsDelivery())
			m_CurrentTargetIsValid = OnHoverDragDropTarget(target, dropContext);
		else
		{
			if (m_CurrentTargetIsValid)
				DropOver(target, dropContext);
			else
				m_Payload.Reset();
		}
	}

}

