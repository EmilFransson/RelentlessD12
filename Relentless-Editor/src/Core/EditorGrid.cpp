#include "EditorGrid.h"
namespace Relentless
{
	EditorGrid::EditorGrid() noexcept
	{
		m_Vertices.at(0).Position = DirectX::XMFLOAT3(-0.5f, 0.0f, 0.0f);
		m_Vertices.at(1).Position = DirectX::XMFLOAT3( 0.5f, 0.0f, 0.0f);

		VertexBuffer::Specification vbSpec{};
		vbSpec.Name = "EditorGridVertexBuffer";
		vbSpec.NrOfVertices = EDITOR_GRID_VERTEX_COUNT;
		vbSpec.Stride = sizeof(Vertex_Basic);
		vbSpec.TotalSizeInBytes = sizeof(Vertex_Basic) * EDITOR_GRID_VERTEX_COUNT;
		vbSpec.pBuffer = m_Vertices.data();
		m_pVertexBuffer = std::make_unique<VertexBuffer>(&vbSpec);

		m_InstanceDataStructuredBuffer = std::make_unique<StructuredBuffer>(EDITOR_GRID_INSTANCE_COUNT, sizeof(InstanceData));
		
		//Only updates for this current frame... (0 because initialization):
		for (int i{ -200 }; i < 200; ++i)
		{
			int index = i + 200;
			InstanceData instanceData{};
			instanceData.Position = DirectX::XMFLOAT3(0.0f, 0.0f, static_cast<float>(i));
			instanceData.Color.R = 10;
			instanceData.Color.G = 10;
			instanceData.Color.B = 10;

			MemoryManager::Get().UpdateStructuredBuffer(*m_InstanceDataStructuredBuffer, &instanceData, index);

			//if (i % 10 == 0)
			//{
			//	m_InstaceData[index].color.r = 23;
			//	m_InstaceData[index].color.g = 23;
			//	m_InstaceData[index].color.b = 23;
			//}
		}
	}
}