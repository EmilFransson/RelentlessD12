#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class ETransformGizmoInteractionState	: uint8 { None = 0,	 Hovering, Using };
	enum class ETransformGizmoType				: int8	{ None = -1, Translate = 0, Rotate = 1, Scale = 2 };
	enum class ETransformGizmoMode				: uint8 { Local = 0, World };
	enum class ETransformGizmoMovementMode		: uint8 { Free = 0, Snap };

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

		[[nodiscard]] ETransformGizmoMode GetActiveMode() const noexcept;
		[[nodiscard]] ETransformGizmoInteractionState GetCurrentInteractionState() const noexcept;
		[[nodiscard]] ETransformGizmoType GetActiveTransformType() const noexcept;

		void SetActiveType(ETransformGizmoType type) noexcept;
		void SetActiveMode(ETransformGizmoMode space) noexcept;
		void SetAllowManipulation(bool state) noexcept;
		void SetActiveTranslationMovementMode(ETransformGizmoMovementMode movementMode) noexcept;
		void SetActiveRotationMovementMode(ETransformGizmoMovementMode movementMode) noexcept;

		void ToggleActiveMode() noexcept;
		void ToggleTranslationMovementMode() noexcept;
		void ToggleRotationMovementMode() noexcept;

		Broadcaster<void(ETransformGizmoInteractionState newState)> OnInteractionStateChanged;
	private:
		void PreConfigureImGuizmo(const TransformGizmoControllerContext& context);
		void Manipulate(const TransformGizmoControllerContext& context) noexcept;
		void DetermineState() noexcept;
	private:
		ETransformGizmoInteractionState m_CurrentState			= ETransformGizmoInteractionState::None;
		ETransformGizmoType				m_CurrentType			= ETransformGizmoType::None;
		ETransformGizmoMode				m_CurrentMode			= ETransformGizmoMode::World;

		ETransformGizmoMovementMode	m_CurrentTranslationMovementMode = ETransformGizmoMovementMode::Free;
		ETransformGizmoMovementMode	m_CurrentRotationMovementMode = ETransformGizmoMovementMode::Free;

		float m_TranslationSnapDelta = 0.1f;
		float m_RotationSnapDelta = 10.0f;
		bool m_AllowManipulation = true;
	};
}