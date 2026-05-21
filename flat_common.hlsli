struct PS_IN
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

#include "constant_buffer_indices.h"
cbuffer CONSTANT_BUFFER : register(FLAT_COMMON_CB)
{
    float4 material_color;
    float2 tile_size;
    float2 texture_size;
    float2 viewport;
    uint y_invert;
    float dummy;
}