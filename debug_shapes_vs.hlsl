#include "debug_shapes.hlsli"

//cbuffer transform : register(b0)
//{
//    row_major float4x4 proj;
//}

struct VS_IN
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

VS_OUT main( VS_IN vin )
{
    VS_OUT vout;
    vout.pos = float4(vin.pos, 1.0f);
    vout.color = vin.color;
	return vout;
}