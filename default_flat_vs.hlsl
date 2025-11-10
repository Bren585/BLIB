#include "flat_vs.hlsli"
VS_OUT main(float4 pos : POSITION, float2 texcoord : TEXCOORD)
{
    VS_OUT vout;
    vout.position   = pos;
    vout.color      = material_color;
    vout.texcoord   = texcoord;
    return vout;
}