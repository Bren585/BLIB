#include "full_vs.hlsli"
VS_OUT main( float4 position : POSITION, float4 normal : NORMAL, float4 tangent : TANGENT, float2 texcoord : TEXCOORD )
{
    VS_OUT vout;
    vout.position       = mul(position, mul(world, view_projection));
    vout.world_position = mul(position, world);
    vout.color          = material_color;
    vout.world_normal   = float4(normalize(normal.xyz), 1); //normalize(mul(normal, world)); need to revert to include rotation
    vout.world_tangent  = float4(normalize(tangent.xyz), tangent.w);
    vout.texcoord       = texcoord;
    
    return vout;
}