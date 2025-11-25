#include "billboard.hlsli"

GS_IN main(float4 position : POSITION, float2 size : TEXCOORD)
{
    GS_IN gin;
    gin.position    = float4(position.xyz, 1);
    gin.size        = size;
    return gin;
}