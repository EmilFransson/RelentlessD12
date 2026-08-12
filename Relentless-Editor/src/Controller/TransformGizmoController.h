#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class ETransformGizmoInteractionState	: uint8 { None = 0,	 Hovering, Using };
	enum class ETransformGizmoType				: int8	{ None = -1, Translate = 0, Rotate = 1, Scale = 2 };
	enum class ETransformGizmoMode				: uint8 { Local = 0, World };
	enum class ETransformGizmoMovementMode		: uint8 { Continuous = 0, Snap };

	struct TransformGizmoControllerContext
	{
		mutable std::vector<entity> Entities;
		Matrix WorldToView;
		Matrix ViewToClip;
		FloatRect Rect;
		bool IsPerspective = true;
		Scene* pScene = nullptr;
	};

	class TransformGizmoController
	{
	public:
		void Execute(const TransformGizmoControllerContext& context) noexcept;

		NO_DISCARD ETransformGizmoMode GetActiveMode() const noexcept;
		NO_DISCARD ETransformGizmoMovementMode GetActiveRotationMovementMode() const noexcept;
		NO_DISCARD ETransformGizmoMovementMode GetActiveScaleMovementMode() const noexcept;
		NO_DISCARD ETransformGizmoMovementMode GetActiveTranslationMovementMode() const noexcept;
		NO_DISCARD ETransformGizmoInteractionState GetCurrentInteractionState() const noexcept;
		NO_DISCARD ETransformGizmoType GetActiveTransformType() const noexcept;
		NO_DISCARD float GetRotationSnapValue() const noexcept;
		NO_DISCARD float GetScaleSnapValue() const noexcept;
		NO_DISCARD float GetTranslationSnapValue() const noexcept;

		NO_DISCARD bool IsInteracting() const noexcept;

		void SetActiveType(ETransformGizmoType type) noexcept;
		void SetActiveMode(ETransformGizmoMode space) noexcept;
		void SetAllowManipulation(bool state) noexcept;
		void SetActiveRotationMovementMode(ETransformGizmoMovementMode aMovementMode) noexcept;
		void SetActiveScaleMovementMode(ETransformGizmoMovementMode aMovementMode) noexcept;
		void SetActiveTranslationMovementMode(ETransformGizmoMovementMode aMovementMode) noexcept;
		void SetRotationSnapValue(float aValue) noexcept;
		void SetScaleSnapValue(float aValue) noexcept;
		void SetTranslationSnapValue(float aValue) noexcept;

		void ToggleActiveMode() noexcept;
		void ToggleRotationMovementMode() noexcept;
		void ToggleScaleMovementMode() noexcept;
		void ToggleTranslationMovementMode() noexcept;

		Broadcaster<void(ETransformGizmoInteractionState newState)> OnInteractionStateChanged;
	private:
		void PreConfigureImGuizmo(const TransformGizmoControllerContext& context);
		void Manipulate(const TransformGizmoControllerContext& context) noexcept;
		void DetermineState() noexcept;
	private:
		ETransformGizmoInteractionState m_CurrentState = ETransformGizmoInteractionState::None;
		ETransformGizmoType	m_CurrentType = ETransformGizmoType::None;
		ETransformGizmoMode	m_CurrentMode = ETransformGizmoMode::World;
		ETransformGizmoMovementMode	m_CurrentTranslationMovementMode = ETransformGizmoMovementMode::Snap;
		ETransformGizmoMovementMode	m_CurrentRotationMovementMode = ETransformGizmoMovementMode::Snap;
		ETransformGizmoMovementMode	m_CurrentScaleMovementMode = ETransformGizmoMovementMode::Snap;

		float m_TranslationSnapDelta = 0.1f;
		float m_RotationSnapDelta = 10.0f;
		float m_ScaleSnapDelta = 0.25f;
		bool m_AllowManipulation = true;
	};
}