#include "full_ps.hlsli"
float4 main(PS_IN pin) : SV_TARGET
{
    return float4(pin.texcoord.x, pin.texcoord.y, 0, 1.0f);
}