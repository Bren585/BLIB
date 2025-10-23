#include "flat_common.hlsli"
Texture2D bump_map : register(t0);
SamplerState pixel_sampler : register(s1);
float4 main(VS_OUT pin) : SV_TARGET
{
    float2 uv = pin.texcoord;
    
    float L = bump_map.Sample(pixel_sampler, uv + float2(-1,  0)).r;
    float R = bump_map.Sample(pixel_sampler, uv + float2( 1,  0)).r;
    float U = bump_map.Sample(pixel_sampler, uv + float2( 0,  1)).r;
    float D = bump_map.Sample(pixel_sampler, uv + float2( 0, -1)).r;
    
    float dx = (R - L) * 0.5;
    float dy = (U - D) * 0.5;
    
    float3 N = normalize(float3(-dx, -dy, 1.0)) * 0.5 + 0.5;
    
    return float4(N, 1.0);
}