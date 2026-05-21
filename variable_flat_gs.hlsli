#include "flat_common.hlsli"

struct GS_IN
{
    float2  position    : POSITION;
    float2  scale       : TEXCOORD0;
    float2  pivot       : TEXCOORD1;
    float   rotation    : TEXCOORD2;
    float2  uv_position : TEXCOORD3;
    float2  uv_size     : TEXCOORD4;
};