#include "math_constants.hlsli"
#include "camera.hlsli"

struct VS_OUT
{
    float4 screen_position  : SV_POSITION;
    //float4 view_position    : VIEW;
    float4 world_position   : POSITION;
    float4 world_normal     : NORMAL;
    float4 world_tangent    : TANGENT;
    float2 texcoord         : TEXCOORD;
    float4 color            : COLOR;
};

