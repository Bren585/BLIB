#include "full_vs.hlsli"

#include "constant_buffer_indices.h"
cbuffer light_perspective : register(SHADOW_CB)
{
    row_major float4x4 light_view_proj;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
    return mul(float4(pos.xyz, 1), mul(world, light_view_proj));
}