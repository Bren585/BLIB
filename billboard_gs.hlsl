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
    
    float3 normal = normalize(position.xyz - camera_position.xyz);
        
    float3 tangent; 
    float3 bitangent;
    {
        float3 up       = float3(0, 1, 0);
        float3 right    = float3(1, 0, 0);
        float3 safe_axis = normalize(lerp(up, right, step(0.999f, abs(normal.y))));
        
        tangent     = normalize(cross(safe_axis,    normal));
        bitangent   = normalize(cross(normal,       tangent));
    }
    
    float3 u = tangent      * extents.x;
    float3 v = bitangent    * extents.y;
    
    vin.normal  = -float4(normal,   1);
    vin.tangent = -float4(tangent,  1);
	
    vin.position = position + float4((-u + v), 0);
    vin.texcoord = float2(0, 0);
    output.Append(default_full_vs(vin));
    
    vin.position = position + float4((u + v), 0);
    vin.texcoord = float2(1, 0);
    output.Append(default_full_vs(vin));
    
    vin.position = position + float4((-u - v), 0);
    vin.texcoord = float2(0, 1);
    output.Append(default_full_vs(vin));
    
    vin.position = position + float4((u - v), 0);
    vin.texcoord = float2(1, 1);
    output.Append(default_full_vs(vin));
    
    output.RestartStrip();

}