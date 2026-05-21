#include "billboard.hlsli"

GS_IN main(float4 position : POSITION, float2 size : TEXCOORD0, float2 uv_size: TEXCOORD1, float2 uv_index: TEXCOORD2)
{
    GS_IN gin;
    gin.position    = float4(position.xyz, 1);
    gin.size        = size;
    gin.uv_size     = uv_size;
    gin.uv_index    = uv_index;
    return gin;
}