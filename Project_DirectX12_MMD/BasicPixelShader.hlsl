#include "BasicShaderHeader.hlsli"
float4 BasicPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(1, -1, 1));
    float brighness = dot(-light, input.normal);
    return float4(brighness,brighness,brighness,1);
}
