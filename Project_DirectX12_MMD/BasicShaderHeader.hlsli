Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);
SamplerState smpToon : register(s1);
Texture2D<float4> sph : register(t1);
Texture2D<float4> spa : register(t2);
Texture2D<float4> toon : register(t3);

cbuffer cbuff : register(b0)
{
    matrix world;//���[���h�ϊ��s��
    matrix view;
    matrix proj;//�r���[�v�v���W�F�N�V�����s��
    float3 eye;
}

cbuffer Material : register(b1)
{
    float4 diffuse;     //�f�B�q���[�Y�F
    float4 specular;    //�X�y�L����
    float3 ambient;     //�A���r�G���g
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