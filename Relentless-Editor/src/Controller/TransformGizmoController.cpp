#include "TransformGizmoController.h"

namespace Relentless
{
	void TransformGizmoController::Execute(const TransformGizmoControllerContext& context) noexcept
	{
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(context.Rect.Left, context.Rect.Top, context.Rect.GetWidth(), context.Rect.GetHeight());

		//Manipulate:
		Scene* pScene = context.pScene;

		const entity pivotEntity = context.Entities.back();

		Matrix pivot = m_CurrentMode == ETransformGizmoMode::World ? pScene->GetWorldTransform(pivotEntity) : pScene->GetLocalTransform(pivotEntity);
		const Matrix pivotNonManipulated = pivot;

		const ImGuizmo::MODE mode = (m_CurrentType == ETransformGizmoType::Scale) ? ImGuizmo::LOCAL : (ImGuizmo::MODE)m_CurrentMode;

		const bool manipulated = ImGuizmo::Manipulate(*context.WorldToView.m, *context.ViewToClip.m, (ImGuizmo::OPERATION)m_CurrentType, mode, pivot.m[0]);
		if (manipulated && m_AllowManipulation) 
		{
			pScene->SetWorldTransform(pivotEntity, pivot);

			const Matrix pivotInverseMatrix = pivotNonManipulated.Invert();

			context.Entities.pop_back();

			for (auto e : context.Entities)
			{
				const Matrix entityWorld = pScene->GetWorldTransform(e);
				const Matrix entityLocalToPivotMatrix = entityWorld * pivotInverseMatrix;
				const Matrix newEntityMatrix = entityLocalToPivotMatrix * pivot;

				pScene->SetWorldTransform(e, newEntityMatrix);
			}
		}

		//Determine new state:
		ETransformGizmoInteractionState state = ETransformGizmoInteractionState::None;

		if (ImGuizmo::IsUsing())
			state = ETransformGizmoInteractionState::Using;
		else if (ImGuizmo::IsOver())
			state = ETransformGizmoInteractionState::Hovering;

		if (state != m_CurrentState)
		{
			m_CurrentState = state;
			OnInteractionStateChanged(m_CurrentState);
		}
	}

	ETransformGizmoMode TransformGizmoController::GetActiveMode() const noexcept
	{
		return m_CurrentMode;
	}

	ETransformGizmoInteractionState TransformGizmoController::GetCurrentInteractionState() const noexcept
	{
		return m_CurrentState;
	}

	ETransformGizmoType TransformGizmoController::GetActiveTransformType() const noexcept
	{
		return m_CurrentType;
	}

	void TransformGizmoController::SetActiveType(ETransformGizmoType type) noexcept
	{
		m_CurrentType = type;
	}

	void TransformGizmoController::SetActiveMode(ETransformGizmoMode mode) noexcept
	{
		m_CurrentMode = mode;
	}

	void TransformGizmoController::SetAllowManipulation(bool state) noexcept
	{
		m_AllowManipulation = state;
	}

	void TransformGizmoController::ToggleActiveMode() noexcept
	{
		if (m_CurrentMode == ETransformGizmoMode::World)
			m_CurrentMode = ETransformGizmoMode::Local;
		else
			m_CurrentMode = ETransformGizmoMode::World;
	}

}