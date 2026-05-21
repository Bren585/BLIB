#include "full_vs.hlsli"

cbuffer BONE_CONSTANT_BUFFER : register(BONE_CB)
{
    row_major float4x4 bone_transforms[BONE_CB_MAX_BONES];
}

struct VS_IN_skinned
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texcoord : TEXCOORD0;
    float4 weights : TEXCOORD1;
    int4 indices : TEXCOORD2;
};