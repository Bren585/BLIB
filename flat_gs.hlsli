#include "flat_common.hlsli"

struct GS_IN
{
    float2  position        : POSITION;
    float2  size            : TEXCOORD0;
    float2  pivot           : TEXCOORD1;
    float   rotation        : TEXCOORD2;
    float2  viewport        : TEXCOORD3;
    uint    y_invert        : TEXCOORD4;
    float2  tile_size       : TEXCOORD5;
    float2  tile_index      : TEXCOORD6;
    float2  texture_size    : TEXCOORD7;    
};