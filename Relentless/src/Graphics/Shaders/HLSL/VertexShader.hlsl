struct Vertex
{
	float3 inPositionLS;
    float3 inNormalLS;
    float2 inTexCoords;
};

struct VS_OUT
{
	float4 outPositionCS	: SV_Position;
    float3 outPositionWS    : POSITIONWS;
    float3 outNormalWS		: NORMALWS;
    float2 outTexCoords		: TEXCOORDS;
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
	
    matrix wvp = mul(vpConstantBuffer.VPMatrix, worldConstantBuffer.worldMatrix);
    vsOut.outPositionCS = mul(wvp, float4(input.inPositionLS, 1.0f));
    vsOut.outPositionWS = mul(worldConstantBuffer.worldMatrix, float4(input.inPositionLS, 1.0f)).xyz;
    vsOut.outNormalWS = mul(worldConstantBuffer.worldMatrix, float4(input.inNormalLS, 0.0f)).xyz;
    vsOut.outTexCoords = input.inTexCoords;
	return vsOut;
}