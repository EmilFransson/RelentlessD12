#pragma once
namespace Relentless
{
	class IDetailLayoutBuilder;

	class IDetailCustomization
	{
	public:
		virtual ~IDetailCustomization() noexcept = default;
		
		virtual void CustomizeDetails(MAYBE_UNUSED IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept {};
		virtual void CustomizeDetails(MAYBE_UNUSED const SharedPtr<IDetailLayoutBuilder>& aDetailLayoutBuilder) noexcept {};

		virtual void OnDestroy(IDetailLayoutBuilder&) noexcept {}

		NO_DISCARD virtual bool ShouldCustomize(IDetailLayoutBuilder&) const noexcept { return true; }
	};
}