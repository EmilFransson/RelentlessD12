#pragma once
namespace Relentless
{
	struct SimpleVertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT3 BiTangent;
		DirectX::XMFLOAT2 TextureCoords;
	};
}