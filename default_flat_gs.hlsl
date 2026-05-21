#include "flat_gs.hlsli"

float2 rotate(float2 p, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    
    return float2
	(
		dot(p, float2(c, -s)), 
		dot(p, float2(s,  c))
	);
}

[maxvertexcount(4)]
void main(
	point GS_IN gin[1], 
	inout TriangleStream<PS_IN> output
)
{
    PS_IN pins[4];
    
    bool	y_invert_b	= y_invert != 0;
    float	y_invert_c	= 1 - float(y_invert_b) * 2;
    float2	position	= gin[0].position;
    position.y = lerp(position.y, (viewport.y - position.y), y_invert_b);
	
    float2 size = tile_size * gin[0].scale;
    size.y *= y_invert_c;
	
    float xflip = step(size.x, 0);
    float yflip = step(size.y, 0);
	float xflip_c = 1 - xflip * 2;
    float yflip_c = 1 - yflip * 2;
	
    //position.x += size.x * xflip;
    //position.y += size.y * yflip;
    size.x *= xflip_c;
    size.y *= yflip_c;
	
    float2 extents = size / 2;
    
    float2 corner[4] =
    {
        float2(-extents.x, -extents.y),
        float2( extents.x, -extents.y),
        float2(-extents.x,  extents.y),
        float2( extents.x,  extents.y)
    };
	
    float2  pivot   = gin[0].pivot * size * 0.5;
    float   angle   = gin[0].rotation * y_invert_c;
    pivot.y *= y_invert_c;
    
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        corner[i] = position + rotate((corner[i] - pivot), angle);
        corner[i].x = 2.0f * corner[i].x / viewport.x - 1.0f;
        corner[i].y = 1.0f - 2.0f * corner[i].y / viewport.y;

        pins[i].position = float4(corner[i], 0, 1);
    }
	
    yflip = lerp(yflip, 1 - yflip, y_invert_b);
    
    float2 tile_index   = gin[0].tile_index;
    float2 uv_transform = tile_size / texture_size;
    
    float u[2] = { 0, 1 };
    float v[2] = { 0, 1 };
   
    pins[0].texcoord = ( float2( u[int(    xflip)], v[int(    yflip)] ) + tile_index ) * uv_transform;
	pins[1].texcoord = ( float2( u[int(1 - xflip)], v[int(    yflip)] ) + tile_index ) * uv_transform;
	pins[2].texcoord = ( float2( u[int(    xflip)], v[int(1 - yflip)] ) + tile_index ) * uv_transform;
	pins[3].texcoord = ( float2( u[int(1 - xflip)], v[int(1 - yflip)] ) + tile_index ) * uv_transform;
		
	[unroll]
    for (int j = 0; j < 4; j++)
    {
        output.Append(pins[j]);
    }
    output.RestartStrip();

}