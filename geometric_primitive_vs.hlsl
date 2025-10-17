#include "full_vs.hlsli"
VS_OUT main( float4 position : POSITION, float4 normal : NORMAL,  float2 texcoord : TEXCOORD )
{
    VS_OUT vout;
    vout.position = mul(position, mul(world, view_projection));
    vout.world_position = mul(position, world);
    
    normal.w = 0;
    float4 N = normalize(mul(normal, world));
    float4 L = normalize(-light_direction);
    
    vout.color.rgb = material_color.rgb * max(0.05, dot(L, N));
    vout.color.a = material_color.a;
    
    vout.texcoord = texcoord;
    vout.world_normal = N;
    
    // Dummy Tangent
    float3 up = abs(N.y) < 0.99 ? float3(0, 1, 0) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, N.xyz));
    vout.world_tangent = float4(tangent, 0);
    
    return vout;
}