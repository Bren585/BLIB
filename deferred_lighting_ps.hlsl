#include "flat_common.hlsli"
#include "shadows.hlsli"
#include "lighting.hlsli"
#include "camera.hlsli"

SamplerState pixel			: register(s1);
SamplerState pixel_clamp	: register(s3);

Texture2D albedo_map	    : register(t0);
Texture2D normal_map        : register(t1);
Texture2D ORM_map		    : register(t2);
Texture2D emissive_map	    : register(t3);
Texture2D position_map      : register(t4);
//StructuredBuffer<light> lights	: register(t5); -> lighting.hlsli
//Texture2DArray shadow_map_array	: register(t6); -> shadows.hlsli

float3 calculate_light_position(float4x4 VP, float3 world_position)
{
    float4 light_space_position = mul(float4(world_position, 1.0), VP);
    light_space_position /= light_space_position.w;
    return light_space_position.xyz;
}

float calculate_sharpness(float3 light_position, float fade)
{
    float d = length(light_position.xy);
    float f = clamp(fade, EPS, 1);
    float e = (1 - f) / f;
    
    return saturate(1 - pow(d, e));
}

float4 main(PS_IN pin) : SV_TARGET
{
    /* Sampling ******************************************************************************/
    
    float4 albedo = albedo_map.Sample(pixel, pin.texcoord);
    if (albedo.a < 0.01)
        discard;
    
    float3 N = normalize(normal_map.Sample(pixel, pin.texcoord).xyz * 2 - 1);
    
    float3 ORM = ORM_map.Sample(pixel, pin.texcoord).rgb;
    float O = ORM.r; // Occlusion
    float R = ORM.g; // Roughness
    float M = ORM.b; // Metallic
    
    float4 E = emissive_map.Sample(pixel, pin.texcoord).rgba; // Emissive
    
    float3 world_position = position_map.Sample(pixel, pin.texcoord).xyz;

    /* View     *****************************************************************************/
    
    float3 V = normalize(camera_position - world_position);
    
    /* Shine    ******************************************************************************/
    
    float3 S = O * albedo.rgb / PI; // Shine
    
    /* Skylight ******************************************************************************/
    
    float3 sky_lighting; 
    {
        float3  L                       = normalize(-skylight_direction.xyz);
        float3  H                       = normalize(V + L);
        float3  light_space_position    = calculate_light_position(skylight_view_proj, world_position);
        float   shadow                  = calculate_shadow(light_space_position, SKYLIGHT_SHADOW_INDEX, N, L);
        float3  sky_radiance            = skylight_color.rgb * skylight_intensity * shadow;
        
        sky_lighting = sky_radiance * calculate_lighting(world_position, albedo.rgb, N, L, V, H, R, M, S);
    }
    
    /* Ambient  ******************************************************************************/
    
    float3 ambient_radiance = ambient_color.rgb * ambient_intensity;
    float3 ambient_lighting = ambient_radiance * S;
    
    /* Lights   ******************************************************************************/
    
    float3 total_lights = { 0, 0, 0 };
    
    [unroll]
    for (int i = 0; i < MAX_LIGHT_COUNT; i++)
    {
        if (i >= light_count)
            break;
        
        light source = lights[i];
        
        float3  L                       = normalize(-source.direction);
        float3  H                       = normalize(V + L);
        float3  light_space_position    = calculate_light_position(source.view_proj, world_position);
        float   sharpness               = calculate_sharpness(light_space_position, source.fade);
        float   shadow                  = calculate_shadow(light_space_position, source.shadow_index, N, L);
        float3  radiance                = source.color.rgb * source.intensity * shadow * sharpness;
        
        total_lights += radiance * calculate_lighting(world_position, albedo.rgb, N, L, V, H, R, M, S);
    }
    
    /* Total    ******************************************************************************/
    
    total_lights += sky_lighting + ambient_lighting + (E.rgb * E.a);
    
    return float4(saturate(total_lights), albedo.a);
}