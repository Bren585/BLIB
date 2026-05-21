#include "lighting_constants.h"

Texture2DArray          shadow_map_array    : register(t6);
SamplerComparisonState  comparison_sampler  : register(s2);

#define PERSPECTIVE_CONST_BIAS  0.0015
#define PERSPECTIVE_SLOPE_SCALE 0.012

#define ORTHO_CONST_BIAS  0.0005
#define ORTHO_SLOPE_SCALE 0.0010

float calculate_shadow(float3 light_position, int shadow_index, float3 N, float3 L, float const_bias, float slope_scale)
{
    float2 uv = light_position.xy * 0.5 + 0.5;
    uv.y = 1 - uv.y;
    float inside = (step(0.0f, uv.x) * step(uv.x, 1.0f) * step(0.0f, uv.y) * step(uv.y, 1.0f));
    
    if (inside < 0.5)
        return 0;
    
    float NdotL = saturate(dot(N, L));
    //float bias = max(0.002 * (1 - NdotL), 0.002);
    float slope = sqrt(1.0 - NdotL * NdotL);
    float bias = const_bias + slope * slope_scale;
    
    return shadow_map_array.SampleCmpLevelZero(comparison_sampler, float3(uv, shadow_index), light_position.z - bias);
    
}