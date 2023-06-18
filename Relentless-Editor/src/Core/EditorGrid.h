#pragma once
#include <Relentless.h>
namespace Relentless
{
	struct Vertex_Basic
	{
		DirectX::XMFLOAT3 Position;
	};

	struct InstanceData
	{
		DirectX::XMFLOAT3 Position;
		float padding1;
		struct
		{
			float R;
			float G;
			float B;
		} Color;
		float padding2;
	};

	class EditorGrid
	{
	public:
		#define EDITOR_GRID_VERTEX_COUNT 2
		#define EDITOR_GRID_INSTANCE_COUNT 400
	public:
		explicit EditorGrid() noexcept;
		~EditorGrid() noexcept = default;
		[[nodiscard]] uint32_t GetVertexCount() const { return EDITOR_GRID_VERTEX_COUNT; }
		[[nodiscard]] uint32_t GetInstanceCount() const { return EDITOR_GRID_INSTANCE_COUNT; }
		std::unique_ptr<VertexBuffer> m_pVertexBuffer;
		std::unique_ptr<StructuredBuffer> m_InstanceDataStructuredBuffer;
	private:
		std::array<Vertex_Basic, EDITOR_GRID_VERTEX_COUNT> m_Vertices;
	};
}