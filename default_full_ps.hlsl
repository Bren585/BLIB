#include "full_ps.hlsli"

SamplerState samplers : register(s0);
SamplerState normal_sampler : register(s1);
Texture2D texture_maps[4] : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = texture_maps[0].Sample(samplers, pin.texcoord) * pin.color;
    if (color.a < 0.01f)
        discard;
    
    float3 N = normalize(pin.world_normal.xyz);
    float3 T = normalize(pin.world_tangent.xyz);
    float sigma = pin.world_tangent.w;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    
    float4 normal = texture_maps[1].Sample(normal_sampler, pin.texcoord);
    normal = (normal * 2.0) - 1.0;
    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
    
    float3 L = normalize(-light_direction.xyz);
    float3 diffuse = color.rgb;// * max(0, dot(N, L));
    
    float3 V = normalize(camera_position.xyz - pin.world_position.xyz);
    float3 specular = pow(max(0, dot(N, normalize(V + L))), 128);
    
    return float4(/*diffuse+*/specular, color.a) + color;
}