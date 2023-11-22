Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

cbuffer cbuff : register(b0)
{
    matrix mat;
}

struct Output
{
    float4 svpos : SV_Position;
    float2 uv : TEXCOORD;
};