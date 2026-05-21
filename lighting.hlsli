#include "math_constants.hlsli"
#include "lighting_constants.h"

#include "constant_buffer_indices.h"
cbuffer CONSTANT_LIGHTING : register(LIGHTING_CB)
{
    int                 light_count;
    
    float3              skylight_direction;
    float3              skylight_color;
    float               skylight_intensity;
    row_major float4x4  skylight_view_proj;
    
    float3              ambient_color;
    float               ambient_intensity;
};

struct light
{
    float3  position;
    float3  direction;
    float3  color;
    float   intensity;
    float   spread;
    float   fade;
    int     shadow_index;
    row_major float4x4 view_proj;
};

StructuredBuffer<light> lights : register(t5);

float3 calculate_lighting(float3 world_position, float3 albedo, float3 N, float3 L, float3 V, float3 H, float R, float M, float3 S)
{
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
    
    /* Diffuse  ******************************************************************************/
    
    float3 diffuse = (1.0 - F) * (1.0 - M); // Diffuse

    return (diffuse * S + specular) * NdotL;
}
