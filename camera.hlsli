#include "constant_buffer_indices.h"
cbuffer CONSTANT_BUFFER : register(CAMERA_CB)
{
    //row_major float4x4  view;
    row_major float4x4  view_projection;
    row_major float4x4  inverse_view_projection;
    //row_major float4x4  inverse_view;
    //row_major float4x4  inverse_projection;
    float3              camera_position;
    float               far_z;
};