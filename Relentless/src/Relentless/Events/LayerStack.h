#pragma once
#include "Layer.h"

namespace Relentless
{
	class LayerStack
	{
	public:
		[[nodiscard]] static LayerStack& Get() noexcept;
		void PushLayer(std::unique_ptr<Layer> pLayer) noexcept;
		void PushOverlay(std::unique_ptr<Layer> pOverlay) noexcept;
		void PopLayer(std::unique_ptr<Layer> pLayer) noexcept;
		void PopOverlay(std::unique_ptr<Layer> pOverlay) noexcept;

		const std::vector<std::unique_ptr<Layer>>::const_iterator begin() const { return m_Layers.begin(); }
		const std::vector<std::unique_ptr<Layer>>::const_iterator end() const { return m_Layers.end(); }
	private:
		LayerStack() noexcept = default;
		~LayerStack() noexcept = default;

	private:
		static LayerStack m_sInstance;
		std::vector<std::unique_ptr<Layer>> m_Layers;
		uint32_t m_LayerInsertIndex = 0;
	};
}