#define MAX_LIGHT_COUNT 16
#define SKYLIGHT_SHADOW_INDEX 1

cbuffer CONSTANT_LIGHTING : register(b0)
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