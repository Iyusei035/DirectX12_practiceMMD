#include "BasicShaderHeader.hlsli"
float4 BasicPS(Output input) : SV_TARGET
{
    //���̌������x�N�g��
    float3 light = normalize(float3(1, -1, 1));
    //���C�g�J���[
    float3 lightColor = float3(1, 1, 1);
    //�f�B�q���[�Y�v�Z
    float diffuseB = saturate(dot(-light, input.normal));
    float4 toonDif = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));
    //���̔��˃x�N�g��
    float3 refLight = normalize(float3(1,-1,1));
    float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);
    
    //�X�t�B�A�}�b�v�pUV
    float2 sphereMapUV = input.vnormal.xy;
    sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5,-0.5);
    
    //�e�N�X�`���J���[
    float4 texColor = tex.Sample(smp, input.uv);
    
    return max(saturate(toonDif //�P�x(�g�D�[��)
		* diffuse //�f�B�t���[�Y�F
		* texColor //�e�N�X�`���J���[
		* sph.Sample(smp, sphereMapUV)) //�X�t�B�A�}�b�v(��Z)
		+ saturate(spa.Sample(smp, sphereMapUV) * texColor //�X�t�B�A�}�b�v(���Z)
		+ float4(specularB * specular.rgb, 1)) //�X�y�L�����[
		, float4(texColor * ambient, 1)); //�A���r�G���g

}
