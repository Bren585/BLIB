#include "skinned_vs.hlsli"

PS_IN main(VS_IN_skinned vin)
{     
    float normal_w = vin.normal.w;
    vin.normal.w = 0;
    float tangent_w = vin.tangent.w;
    vin.tangent.w = 0;
    
    float4 blended_position = { 0, 0, 0, 1 };
    float4 blended_normal = { 0, 0, 0, 0 };
    float4 blended_tangent = { 0, 0, 0, 0 };
    
    [unroll]
    for (int bone_index = 0; bone_index < 4; bone_index++)
    {
        float3x3 bone_normal = transpose(inverse((float3x3) bone_transforms[vin.indices[bone_index]]));
        blended_position += vin.weights[bone_index] * mul(vin.position, bone_transforms[vin.indices[bone_index]]);
        blended_normal += vin.weights[bone_index] * float4(mul(vin.normal.xyz, bone_normal), 0);
        blended_tangent += vin.weights[bone_index] * float4(mul(vin.tangent.xyz, bone_normal), 0);
    }
    vin.position = float4(blended_position.xyz, 1);
    vin.normal = float4(blended_normal.xyz, normal_w);
    vin.tangent = float4(blended_tangent.xyz, tangent_w);
    
    float3x3 normal_world = transpose(inverse((float3x3) world));
    
    PS_IN vout;
    vout.screen_position = mul(vin.position, mul(world, view_projection));
    vout.world_position = mul(vin.position, world);
    vout.world_normal = float4(normalize(mul(vin.normal.xyz, normal_world)), vin.normal.w);
    vout.world_tangent = float4(normalize(mul(vin.tangent.xyz, normal_world)), vin.tangent.w);
    vout.color = material_color;
    vout.texcoord = vin.texcoord;
    
    return vout;
}