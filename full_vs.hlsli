#include "full_common.hlsli"
#include "inverse.hlsli"

#include "constant_buffer_indices.h"
cbuffer CONSTANT_BUFFER : register(FULL_VS_CB)
{
    row_major float4x4 world;
    float4 material_color;
    //float2 uv_size;
    //float2 uv_index;
};

struct VS_IN
{
    float4 position : POSITION;
    float4 normal   : NORMAL;
    float4 tangent  : TANGENT;
    float2 texcoord : TEXCOORD0;
};

PS_IN default_full_vs(VS_IN vin)
{
    float3x3 normal_world = transpose(inverse((float3x3) world));
    
    PS_IN vout;
    vout.screen_position    = mul(vin.position, mul(world, view_projection));
    //vout.view_position      = mul(position, mul(world, view));
    vout.world_position     = mul(vin.position, world);
    vout.color              = material_color;
    vout.world_normal       = float4(normalize(mul(vin.normal.xyz,  normal_world)), vin.normal.w);
    vout.world_tangent      = float4(normalize(mul(vin.tangent.xyz, normal_world)), vin.tangent.w);
    vout.texcoord           = vin.texcoord; // + uv_index) * uv_size;
    
    return vout;
}