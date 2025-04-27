#include "TransformGizmoController.h"

namespace Relentless
{
	namespace TransformGizmoController_private
	{
		template<typename EnumType>
		void ToggleEnum(EnumType& e, EnumType first, EnumType second)
		{
			e = (e == first) ? second : first;
		}

		void FillSnap(float(&out)[3], float value)
		{
			out[0] = out[1] = out[2] = value;
		}
	}

	void TransformGizmoController::Execute(const TransformGizmoControllerContext& context) noexcept
	{
		PreConfigureImGuizmo(context);
		Manipulate(context);
		DetermineState();
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

	void TransformGizmoController::SetActiveTranslationMovementMode(ETransformGizmoMovementMode movementMode) noexcept
	{
		m_CurrentTranslationMovementMode = movementMode;
	}

	void TransformGizmoController::SetActiveRotationMovementMode(ETransformGizmoMovementMode movementMode) noexcept
	{
		m_CurrentRotationMovementMode = movementMode;
	}

	void TransformGizmoController::ToggleActiveMode() noexcept
	{
		TransformGizmoController_private::ToggleEnum(m_CurrentMode, ETransformGizmoMode::World, ETransformGizmoMode::Local);
	}

	void TransformGizmoController::ToggleTranslationMovementMode() noexcept
	{
		TransformGizmoController_private::ToggleEnum(m_CurrentTranslationMovementMode, ETransformGizmoMovementMode::Free, ETransformGizmoMovementMode::Snap);
	}

	void TransformGizmoController::ToggleRotationMovementMode() noexcept
	{
		TransformGizmoController_private::ToggleEnum(m_CurrentRotationMovementMode, ETransformGizmoMovementMode::Free, ETransformGizmoMovementMode::Snap);
	}

	void TransformGizmoController::PreConfigureImGuizmo(const TransformGizmoControllerContext& context)
	{
		ImGuizmo::SetOrthographic(!context.IsPerspective);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(context.Rect.Left, context.Rect.Top, context.Rect.GetWidth(), context.Rect.GetHeight());
	}

	void TransformGizmoController::Manipulate(const TransformGizmoControllerContext& context) noexcept
	{
		if (context.Entities.empty())
			return;

		std::vector<entity> entities = context.Entities;

		Scene* pScene = context.pScene;
		const entity pivotEntity = entities.back();

		Matrix pivot = m_CurrentMode == ETransformGizmoMode::World ? pScene->GetWorldTransform(pivotEntity) : pScene->GetLocalTransform(pivotEntity);
		const Matrix pivotNonManipulated = pivot;

		const ImGuizmo::MODE mode = (m_CurrentType == ETransformGizmoType::Scale) ? ImGuizmo::LOCAL : (ImGuizmo::MODE)m_CurrentMode;

		float snapValues[3];
		if (m_CurrentType == ETransformGizmoType::Translate)
			TransformGizmoController_private::FillSnap(snapValues, m_TranslationSnapDelta);
		else if (m_CurrentType == ETransformGizmoType::Rotate)
			TransformGizmoController_private::FillSnap(snapValues, m_RotationSnapDelta);

		float* pSnapDelta = nullptr;
		if ((m_CurrentType == ETransformGizmoType::Translate && m_CurrentTranslationMovementMode == ETransformGizmoMovementMode::Snap)
			|| m_CurrentType == ETransformGizmoType::Rotate && m_CurrentRotationMovementMode == ETransformGizmoMovementMode::Snap)
		{
			pSnapDelta = snapValues;
		}

		const bool manipulated = ImGuizmo::Manipulate(*context.WorldToView.m, *context.ViewToClip.m, (ImGuizmo::OPERATION)m_CurrentType, mode, pivot.m[0], nullptr, pSnapDelta);
		if (manipulated && m_AllowManipulation)
		{
			pScene->SetWorldTransform(pivotEntity, pivot);

			const Matrix pivotInverseMatrix = pivotNonManipulated.Invert();

			entities.pop_back();

			for (auto e : entities)
			{
				const Matrix entityWorld = pScene->GetWorldTransform(e);
				const Matrix entityLocalToPivotMatrix = entityWorld * pivotInverseMatrix;
				const Matrix newEntityMatrix = entityLocalToPivotMatrix * pivot;

				pScene->SetWorldTransform(e, newEntityMatrix);
			}
		}
	}

	void TransformGizmoController::DetermineState() noexcept
	{
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
}