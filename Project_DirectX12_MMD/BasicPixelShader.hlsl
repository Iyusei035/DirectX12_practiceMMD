#include "BasicShaderHeader.hlsli"
float4 BasicPS(Output input) : SV_TARGET
{
    //光の向かうベクトル
    float3 light = normalize(float3(1, -1, 1));
    //ライトカラー
    float3 lightColor = float3(1, 1, 1);
    //ディヒューズ計算
    float diffuseB = saturate(dot(-light, input.normal));
    float4 toonDif = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));
    //光の反射ベクトル
    float3 refLight = normalize(float3(1,-1,1));
    float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);
    
    //スフィアマップ用UV
    float2 sphereMapUV = input.vnormal.xy;
    sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5,-0.5);
    
    //テクスチャカラー
    float4 texColor = tex.Sample(smp, input.uv);
    
    return max(saturate(toonDif //輝度(トゥーン)
		* diffuse //ディフューズ色
		* texColor //テクスチャカラー
		* sph.Sample(smp, sphereMapUV)) //スフィアマップ(乗算)
		+ saturate(spa.Sample(smp, sphereMapUV) * texColor //スフィアマップ(加算)
		+ float4(specularB * specular.rgb, 1)) //スペキュラー
		, float4(texColor * ambient, 1)); //アンビエント

}
