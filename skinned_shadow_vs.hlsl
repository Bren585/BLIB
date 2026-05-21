#include "skinned_vs.hlsli"

#include "constant_buffer_indices.h"
cbuffer light_perspective : register(SHADOW_CB)
{
    row_major float4x4 light_view_proj;
}

float4 main(VS_IN_skinned vin) : SV_Position
{    
    float4 blended_position = { 0, 0, 0, 1 };
    
    [unroll]
    for (int bone_index = 0; bone_index < 4; bone_index++)
    {
        blended_position += vin.weights[bone_index] * mul(vin.position, bone_transforms[vin.indices[bone_index]]);
    }
    vin.position = float4(blended_position.xyz, 1);
    
    return mul(vin.position, mul(world, light_view_proj));
}