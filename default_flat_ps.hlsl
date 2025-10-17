#include "flat_common.hlsli"
Texture2D color_map : register(t0);
SamplerState samplers : register(s0);
float4 main(VS_OUT pin) : SV_TARGET
{
    return color_map.Sample(samplers, pin.texcoord) * pin.color;
}