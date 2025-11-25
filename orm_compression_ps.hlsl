#include "flat_common.hlsli"

Texture2D O : register(t0);
Texture2D R : register(t1);
Texture2D M : register(t2);
SamplerState pixel_sampler : register(s1);

float4 main(PS_IN pin) : SV_TARGET
{
    float2 pos = pin.texcoord;
    return float4(O.Sample(pixel_sampler, pos).r, R.Sample(pixel_sampler, pos).g, M.Sample(pixel_sampler, pos).b, 1.0f);
}