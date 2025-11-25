#include "full_ps.hlsli"

SamplerState    samplers        : register(s0);
SamplerState    pixel_sampler   : register(s1);

Texture2D       texture_map     : register(t0);
Texture2D       normal_map      : register(t1);
Texture2D       orm_map         : register(t2);
Texture2D       emissive_map    : register(t3);

PS_OUT main(PS_IN pin)
{
    PS_OUT pout;
    
    /* Albedo ********************************************************************************/
    
    float4 albedo = texture_map.Sample(samplers, pin.texcoord) * pin.color;
    if (albedo.a < 1.0)
        discard;
    
    pout.slot0 = albedo;
    
    /* Normals  ******************************************************************************/
    
    float3 N = normalize(pin.world_normal.xyz); // Geometric Normal
    {
        float3 T = normalize(pin.world_tangent.xyz); // Tangent
        T = normalize(T - N * dot(N, T));
        float3 B = normalize(cross(N, T)) * pin.world_tangent.w; // Bitangent
        float3 n = normal_map.Sample(pixel_sampler, pin.texcoord).rgb * 2 - 1; // Surface Normal
        N = normalize((n.x * T) + (n.y * B) + (n.z * N));
    }
    
    pout.slot1 = float4(N * 0.5 + 0.5, 1);
    
    /* ORM       *****************************************************************************/
    
    pout.slot2 = float4(orm_map.Sample(pixel_sampler, pin.texcoord).rgb, 1);
    
    /* Emissive  *****************************************************************************/
    
    pout.slot3 = emissive_map.Sample(pixel_sampler, pin.texcoord).rgba;
    
    /* Depth     *****************************************************************************/
    
    //float depth = pin.view_position.z / far_z;
    //pout.slot4 = float4(depth, depth, depth, 1);
    
    pout.slot4 = float4(pin.world_position.xyz, 1);
    
    /* Finalize  *****************************************************************************/
    
    pout.slot5 = float4(0, 0, 0, 0);
    pout.slot6 = float4(0, 0, 0, 0);
    pout.slot7 = float4(0, 0, 0, 0);
    
    return pout;
}