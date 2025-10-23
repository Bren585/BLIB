#include "math_constants.hlsli"

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float4 world_normal : NORMAL;
    float4 world_tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 view_projection;
    float4 light_direction;
    float4 light_color;
    float4 ambient_color;
    float4 camera_position;
};

#define light_intensity light_color.a
#define ambient_intensity ambient_color.a