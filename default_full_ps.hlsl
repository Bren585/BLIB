#include "full_ps.hlsli"
#include "lighting.hlsli"

SamplerState samplers       : register(s0);
SamplerState pixel_sampler  : register(s1);
Texture2D texture_map       : register(t0);
Texture2D normal_map        : register(t1);
Texture2D orm_map           : register(t2);
Texture2D emissive_map      : register(t3);

PS_OUT main(VS_OUT pin)
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
    
    /* Constants *****************************************************************************/
    
    float3 L = normalize(-skylight_direction.xyz); // Light Direction
    float3 V = normalize(camera_position.xyz - pin.world_position.xyz); // View Direction
    float3 H = normalize(V + L);
        
    /* Normals  ******************************************************************************/
    
    float3 N = normalize(pin.world_normal.xyz); // Geometric Normal
    {
        float3 T = normalize(pin.world_tangent.xyz); // Tangent
        T = normalize(T - N * dot(N, T));
        float3 B = normalize(cross(N, T)) * pin.world_tangent.w; // Bitangent
        float3 n = normal_map.Sample(pixel_sampler, pin.texcoord).rgb * 2 - 1; // Surface Normal
        N = normalize((n.x * T) + (n.y * B) + (n.z * N));
    }
    
    /* Precalcs ******************************************************************************/
    
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));
    
    /* Fresnel  ******************************************************************************/
    
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo.rgb, M);
    float3 F = F0 + ((1 - F0) * pow(1.0 - VdotH, 5.0));
    
    /* GGX      ******************************************************************************/
    
    float D; // Distribution
    {
        float a = R * R;
        float d = (NdotH * NdotH) * (a - 1.0) + 1.0;
        D = a / (PI * d * d);
    }
    
    /* Geometry ******************************************************************************/
    
    float G; // Geometry
    {
        float k = R + 1.0;
        k = (k * k) / 8.0;
        float ki = (1 - k);
        float Gv = NdotV / (NdotV * ki + k);
        float Gl = NdotL / (NdotL * ki + k);
        G = Gv * Gl;
    }
    
    /* Specular ******************************************************************************/
    
    float3 specular; // Specular
    {
        float3 n = D * F * G;
        float d = 4.0 * NdotV * NdotL + 0.001;
        specular = n / d;
    }
    
    /* Shine    ******************************************************************************/
    
    float3 S = O * albedo.rgb / PI; // Shine
    
    /* Diffuse  ******************************************************************************/
    
    float3 diffuse = (1.0 - F) * (1.0 - M); // Diffuse
    
    /* Lighting ******************************************************************************/
    
    float3 sky_radiance = skylight_color.rgb * skylight_intensity;
    float3 sky_lighting = (diffuse * S + specular) * sky_radiance * NdotL;
    
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