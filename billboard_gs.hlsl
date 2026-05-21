#include "billboard.hlsli"
#include "full_vs.hlsli"

[maxvertexcount(4)]
void main(
	point GS_IN gin[1], 
	inout TriangleStream<PS_IN> output
)
{
    VS_IN vin;
    float4 position = gin[0].position;
    float2 extents  = gin[0].size * 0.5;
    float2 uv_size  = gin[0].uv_size;
    float2 uv_index = gin[0].uv_index;
    
#if 0// World Upright
    float3 normal = normalize(camera_position.xyz - mul(position, world).xyz);
       
    float3 tangent; 
    float3 bitangent;
    {
        float3 up       = float3(0, 1, 0);
        float3 right    = float3(1, 0, 0);
        float3 safe_axis = normalize(lerp(up, right, step(0.999f, abs(normal.y))));
        
        tangent     = normalize(cross(safe_axis,    normal));
        bitangent   = normalize(cross(normal,       tangent));
    }
#else
    float3 tangent      = normalize( inverse_view_projection[0].xyz);
    float3 bitangent    = normalize( inverse_view_projection[1].xyz);
    float3 normal       = normalize(-inverse_view_projection[2].xyz);
#endif
    
    float3 u = tangent      * extents.x;
    float3 v = bitangent    * extents.y;
    
    vin.normal  = float4(normal,   1);
    vin.tangent = float4(tangent,  1);
	
    vin.position = position + float4((-u + v), 0);
    vin.texcoord = (float2(0, 0) + uv_index) * uv_size;
    output.Append(default_full_vs(vin));
    
    vin.position = position + float4((u + v), 0);
    vin.texcoord = (float2(1, 0) + uv_index) * uv_size;
    output.Append(default_full_vs(vin));
    
    vin.position = position + float4((-u - v), 0);
    vin.texcoord = (float2(0, 1) + uv_index) * uv_size;
    output.Append(default_full_vs(vin));
    
    vin.position = position + float4((u - v), 0);
    vin.texcoord = (float2(1, 1) + uv_index) * uv_size;
    output.Append(default_full_vs(vin));
    
    output.RestartStrip();

}