#include "full_vs.hlsli"

cbuffer light_perspective : register(b1)
{
    row_major float4x4 light_view_proj;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
    return mul(float4(pos.xyz, 1), mul(world, light_view_proj));
}