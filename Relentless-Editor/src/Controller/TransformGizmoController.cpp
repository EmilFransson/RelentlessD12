#include "TransformGizmoController.h"

#include "Core/Editor.h"

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

		static ImGuizmo::OPERATION ToImGuizmoOperation(ETransformGizmoType aType) noexcept
		{
			switch (aType)
			{
			case ETransformGizmoType::Translate:
				return ImGuizmo::OPERATION::TRANSLATE;
			case ETransformGizmoType::Rotate:
				return ImGuizmo::OPERATION::ROTATE;
			case ETransformGizmoType::Scale:
				return ImGuizmo::OPERATION::SCALE;
			default:
				return ImGuizmo::OPERATION::UNIVERSAL;
			}
		}

		static ImGuizmo::MODE ToImGuizmoMode(ETransformGizmoMode aMode) noexcept
		{
			switch (aMode)
			{
			case ETransformGizmoMode::Local: 
				return ImGuizmo::LOCAL;
			case ETransformGizmoMode::World: 
				return ImGuizmo::WORLD;
			default: 
				return ImGuizmo::LOCAL;
			}
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

	bool TransformGizmoController::IsInteracting() const noexcept
	{
		return GetActiveTransformType() != ETransformGizmoType::None && (ImGuizmo::IsOver() || ImGuizmo::IsUsing());
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
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::SetRect(context.Rect.Left, context.Rect.Top, context.Rect.GetWidth(), context.Rect.GetHeight());
	}

	void TransformGizmoController::Manipulate(const TransformGizmoControllerContext& context) noexcept
	{
		if (context.Entities.empty())
			return;

		if (m_CurrentType == ETransformGizmoType::None)
			return;

		Scene* pScene = context.pScene;
		EntityManager& entityManager = pScene->GetEntityManager();

		// Copy selection so we can pop_back etc
		std::vector<entity> entities = context.Entities;
		const entity pivotEntity = entities.back();

		// ---------------------------------------------------------------------
		// 1) Cache ORIGINAL world matrices for all selected entities
		//    This is critical so we don't mix "before" and "after" transforms
		// ---------------------------------------------------------------------
		std::unordered_map<entity, Matrix> originalWorld;
		originalWorld.reserve(entities.size());

		for (entity e : entities)
			originalWorld.emplace(e, entityManager.Get<TransformComponent>(e).GetWorldMatrix());

		// Use the ORIGINAL pivot matrix as input to ImGuizmo
		Matrix pivot = originalWorld[pivotEntity];
		const Matrix pivotOriginal = pivot; // keep a copy explicitly

		// ImGuizmo mode: LOCAL/WORLD only affects orientation, not what matrix we pass
		const ImGuizmo::MODE mode = (m_CurrentType == ETransformGizmoType::Scale) ? ImGuizmo::LOCAL : TransformGizmoController_private::ToImGuizmoMode(m_CurrentMode);

		// Snap setup
		float snapValues[3];
		if (m_CurrentType == ETransformGizmoType::Translate)
			TransformGizmoController_private::FillSnap(snapValues, m_TranslationSnapDelta);
		else if (m_CurrentType == ETransformGizmoType::Rotate)
			TransformGizmoController_private::FillSnap(snapValues, m_RotationSnapDelta);

		float* pSnapDelta = nullptr;
		if ((m_CurrentType == ETransformGizmoType::Translate && m_CurrentTranslationMovementMode == ETransformGizmoMovementMode::Snap) ||
			(m_CurrentType == ETransformGizmoType::Rotate && m_CurrentRotationMovementMode == ETransformGizmoMovementMode::Snap))
		{
			pSnapDelta = snapValues;
		}

		// ---------------------------------------------------------------------
		// 2) Let ImGuizmo modify the pivot matrix IN PLACE
		// ---------------------------------------------------------------------
		const bool manipulated = ImGuizmo::Manipulate(
			*context.WorldToView.m,
			*context.ViewToClip.m,
			TransformGizmoController_private::ToImGuizmoOperation(m_CurrentType),
			mode,
			pivot.m[0],       // IN: original pivot world, OUT: new pivot world
			nullptr,
			pSnapDelta
		);

		if (!manipulated || !m_AllowManipulation)
			return;

		// ---------------------------------------------------------------------
		// 3) Decompose the NEW pivot matrix and apply to pivot entity (world space)
		// ---------------------------------------------------------------------
		Vector3    scale = Vector3::One;
		Quaternion rotation = Quaternion::Identity;
		Vector3    location = Vector3::Zero;

		if (!pivot.Decompose(scale, rotation, location))
			return;

		{
			auto& pivotTC = entityManager.Get<TransformComponent>(pivotEntity);
			pivotTC.SetWorldScale(scale);
			pivotTC.SetWorldRotation(rotation);
			pivotTC.SetWorldLocation(location);

			Editor::OnEntityTransformed(pivotEntity);
		}

		// ---------------------------------------------------------------------
		// 4) Compute a clean delta matrix: oldPivot -> newPivot
		//
		//     P0 = pivotOriginal
		//     P1 = pivot (new)
		//     delta = P0^-1 * P1
		//
		//  Then for each entity:
		//     E0' = E0 * delta
		// ---------------------------------------------------------------------
		const Matrix pivotOriginalInv = pivotOriginal.Invert();
		const Matrix delta = pivotOriginalInv * pivot;

		// We already handled the pivot entity itself
		entities.pop_back();

		// ---------------------------------------------------------------------
		// 5) Apply the SAME delta to each other selected entity,
		//    using their ORIGINAL world matrix (from before any changes).
		// ---------------------------------------------------------------------
		for (entity e : entities)
		{
			auto it = originalWorld.find(e);
			if (it == originalWorld.end())
				continue;

			const Matrix& entityWorldOriginal = it->second;
			Matrix  newEntityWorld = entityWorldOriginal * delta;

			Vector3    eScale;
			Quaternion eRotation;
			Vector3    eLocation;

			if (!newEntityWorld.Decompose(eScale, eRotation, eLocation))
				continue;

			auto& entityTC = entityManager.Get<TransformComponent>(e);
			entityTC.SetWorldScale(eScale);
			entityTC.SetWorldRotation(eRotation);
			entityTC.SetWorldLocation(eLocation);

			Editor::OnEntityTransformed(e);
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