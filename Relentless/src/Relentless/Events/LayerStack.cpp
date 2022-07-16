#include "LayerStack.h"
#include "Layer.h"

namespace Relentless
{
	LayerStack LayerStack::m_sInstance;

	LayerStack& LayerStack::Get() noexcept
	{
		return m_sInstance;
	}

	void LayerStack::PushLayer(std::unique_ptr<Layer> pLayer) noexcept
	{
		pLayer->OnAttach();
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, std::move(pLayer));
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(std::unique_ptr<Layer> pOverlay) noexcept
	{
		pOverlay->OnAttach();
		m_Layers.emplace_back(std::move(pOverlay));
	}

	void LayerStack::PopLayer(std::unique_ptr<Layer> pLayer) noexcept
	{
		auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, pLayer);
		if (it != m_Layers.begin() + m_LayerInsertIndex)
		{
			pLayer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(std::unique_ptr<Layer> pOverLay) noexcept
	{
		auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), pOverLay);
		if (it != m_Layers.end())
		{
			pOverLay->OnDetach();
			m_Layers.erase(it);
		}
	}
}