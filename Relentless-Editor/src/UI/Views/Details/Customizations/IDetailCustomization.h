#pragma once
namespace Relentless
{
	class IDetailLayoutBuilder;

	class IDetailCustomization
	{
	public:
		virtual ~IDetailCustomization() noexcept = default;
		
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept = 0;

		virtual void OnDestroy(IDetailLayoutBuilder&) noexcept {}

		NO_DISCARD virtual bool ShouldCustomize(IDetailLayoutBuilder&) const noexcept { return true; }
	};
}