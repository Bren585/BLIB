#include "full_ps.hlsli"
#include "lighting.hlsli"

SamplerState samplers       : register(s0);
SamplerState pixel_sampler  : register(s1);
Texture2D texture_map       : register(t0);
Texture2D normal_map        : register(t1);
Texture2D orm_map           : register(t2);
Texture2D emissive_map      : register(t3);

PS_OUT main(PS_IN pin)
{
    PS_OUT pout;
    
    /* Sampling ******************************************************************************/
    
    float4 albedo = texture_map.Sample(samplers, pin.texcoord) * pin.color;
    if (albedo.a < 0.01)
        discard;
    
    float3 ORM = orm_map.Sample(pixel_sampler, pin.texcoord).rgb;
    float O = ORM.r; // Occlusion
    float R = ORM.g; // Roughness
    float M = ORM.b; // Metallic
    
    float4 E = emissive_map.Sample(pixel_sampler, pin.texcoord).rgba; // Emissive
        
    /* Normals  ******************************************************************************/
    
    float3 N = normalize(pin.world_normal.xyz); // Geometric Normal
    {
        float3 T = normalize(pin.world_tangent.xyz); // Tangent
        T = normalize(T - N * dot(N, T));
        float3 B = normalize(cross(N, T)) * pin.world_tangent.w; // Bitangent
        float3 n = normal_map.Sample(pixel_sampler, pin.texcoord).rgb * 2 - 1; // Surface Normal
        N = normalize((n.x * T) + (n.y * B) + (n.z * N));
    }
    
    /* Shine    ******************************************************************************/
    
    float3 S = O * albedo.rgb / PI; // Shine
    
    /* Skylight ******************************************************************************/
    
    float3 sky_lighting;
    {
        float3 V = normalize(camera_position.xyz - pin.world_position.xyz);
        float3 L = normalize(-skylight_direction.xyz);
        float3 H = normalize(V + L);
        float3 sky_radiance = skylight_color.rgb * skylight_intensity;
        
        sky_lighting = sky_radiance * calculate_lighting(pin.world_position.xyz, albedo.rgb, N, L, V, H, R, M, S);
    }
    
    /* Ambient  ******************************************************************************/
    
    float3 ambient_radiance = ambient_color.rgb * ambient_intensity;
    float3 ambient_lighting = ambient_radiance * S;
    
    float3 total_lighting = sky_lighting + ambient_lighting + (E.rgb * E.a);
    
    pout.slot0 = float4(saturate(total_lighting), albedo.a); // default pass
    
    pout.slot1 = float4(0, 0, 0, 0);
    pout.slot2 = float4(0, 0, 0, 0);
    pout.slot3 = float4(0, 0, 0, 0);
    pout.slot4 = float4(0, 0, 0, 0);
    pout.slot5 = float4(0, 0, 0, 0);
    pout.slot6 = float4(0, 0, 0, 0);
    pout.slot7 = float4(0, 0, 0, 0);
    
    return pout;
}