#include "full_vs.hlsli"
#include "inverse.hlsli"
VS_OUT main( float4 position : POSITION, float4 normal : NORMAL, float4 tangent : TANGENT, float2 texcoord : TEXCOORD )
{ 
    float3x3 normal_world = transpose(inverse((float3x3) world));
    
    VS_OUT vout;
    vout.screen_position    = mul(position, mul(world, view_projection));
    //vout.view_position      = mul(position, mul(world, view));
    vout.world_position     = mul(position, world);
    vout.color              = material_color;
    vout.world_normal       = float4(normalize(mul(normal.xyz,  normal_world)), normal.w);
    vout.world_tangent      = float4(normalize(mul(tangent.xyz, normal_world)), tangent.w);
    vout.texcoord           = texcoord;
    
    return vout;
}