#include "flat_ps.hlsli"
Texture2D color_map : register(t0);
SamplerState samplers : register(s0);
float4 main(PS_IN pin) : SV_TARGET
{
    return color_map.Sample(samplers, pin.texcoord) * material_color;
}