struct Vertex
{
	float3 inPositionLS;
};

struct VS_OUT
{
	float4 outPositionCS	: SV_Position;
};

StructuredBuffer<Vertex> vertices : register(t0, space0);
StructuredBuffer<unsigned int> indices: register(t1, space0);

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
	VS_OUT vsOut = (VS_OUT)0;

	Vertex input = vertices[indices[vertexID]];
	vsOut.outPositionCS = float4(input.inPositionLS, 1.0f);
	return vsOut;
}