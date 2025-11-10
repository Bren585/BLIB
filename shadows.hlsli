Texture2DArray shadow_map_array : register(t6);
SamplerComparisonState comparison_sampler : register(s2);

float calculate_shadow(float3 light_position, int shadow_index, float3 N, float3 L)
{
    float NdotL = dot(N, L);    
    float bias = max(0.002 * (1 - NdotL), 0.002);
    float2 uv = light_position.xy * 0.5 + 0.5;
    uv.y = 1 - uv.y;
    float inside = 
            step(0.0f, uv.x) * step(uv.x, 1.0f) *
            step(0.0f, uv.y) * step(uv.y, 1.0f) *
            step(0.0f, light_position.z) * step(light_position.z, 1.0f);
    
    return shadow_map_array.SampleCmpLevelZero(comparison_sampler, float3(uv, shadow_index), light_position.z - bias) * inside;
}