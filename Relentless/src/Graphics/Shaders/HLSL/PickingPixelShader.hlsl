struct PS_IN
{
    float4 inPositionSS : SV_Position;
};

struct Identity
{
    uint ID;
};

ConstantBuffer<Identity> Identifier : register(b0, space1);

uint ps_main() : SV_TARGET
{
    return Identifier.ID;
}