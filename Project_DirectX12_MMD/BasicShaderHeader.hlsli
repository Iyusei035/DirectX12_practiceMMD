Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);
SamplerState smpToon : register(s1);
Texture2D<float4> sph : register(t1);
Texture2D<float4> spa : register(t2);
Texture2D<float4> toon : register(t3);

cbuffer cbuff : register(b0)
{
    matrix world;//ワールド変換行列
    matrix view;
    matrix proj;//ビューププロジェクション行列
    float3 eye;
}

cbuffer Material : register(b1)
{
    float4 diffuse;     //ディヒューズ色
    float4 specular;    //スペキュラ
    float3 ambient;     //アンビエント
}

struct Output
{
    float4 svpos : SV_POSITION;
    float4 pos : POSITION;
    float4 normal : NORMAL0;
    float4 vnormal : NORMAL1;
    float2 uv : TEXCOORD;
    float3 ray : VECTOR;
};