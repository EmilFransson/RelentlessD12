#include <Relentless.h>

namespace Relentless
{
	enum class ETransformGizmoInteractionState	: uint8 { None = 0,	 Hovering, Using };
	enum class ETransformGizmoType				: int8	{ None = -1, Translate = 0, Rotate = 1, Scale = 2 };
	enum class ETransformGizmoMode				: uint8 { Local = 0, World };

	struct TransformGizmoControllerContext
	{
		mutable std::vector<entity> Entities;
		Matrix WorldToView;
		Matrix ViewToClip;
		FloatRect Rect;
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

		void ToggleActiveMode() noexcept;

		Broadcaster<void(ETransformGizmoInteractionState newState)> OnInteractionStateChanged;
	private:
		ETransformGizmoInteractionState m_CurrentState	= ETransformGizmoInteractionState::None;
		ETransformGizmoType m_CurrentType				= ETransformGizmoType::None;
		ETransformGizmoMode m_CurrentMode				= ETransformGizmoMode::World;

		bool m_AllowManipulation = true;
	};
}