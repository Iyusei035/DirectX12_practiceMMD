Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

cbuffer cbuff : register(b0)
{
    matrix world;//���[���h�ϊ��s��
    matrix viewproj;//�r���[�v�v���W�F�N�V�����s��
}

struct Output
{
    float4 svpos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};