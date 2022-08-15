struct Vertex
{
	float3 inPositionLS;
};

struct VS_OUT
{
	float4 outPositionCS	: SV_Position;
};

struct VPConstantBuffer
{
	matrix VPMatrix;
};

struct WorldConstantBuffer
{
	matrix worldMatrix;
};

ConstantBuffer<VPConstantBuffer> vpConstantBuffer : register(b0, space0);
ConstantBuffer<WorldConstantBuffer> worldConstantBuffer : register(b1, space0);
StructuredBuffer<Vertex> vertices : register(t0, space0);
StructuredBuffer<unsigned int> indices: register(t1, space0);

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
	VS_OUT vsOut = (VS_OUT)0;
	Vertex input = vertices[indices[vertexID]];
	
	matrix wvp = mul(worldConstantBuffer.worldMatrix, vpConstantBuffer.VPMatrix);
	vsOut.outPositionCS = mul(float4(input.inPositionLS, 1.0f), wvp);
	return vsOut;
}