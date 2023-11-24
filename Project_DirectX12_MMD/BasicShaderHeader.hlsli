Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

cbuffer cbuff : register(b0)
{
    matrix world;//ワールド変換行列
    matrix viewproj;//ビューププロジェクション行列
}

struct Output
{
    float4 svpos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};