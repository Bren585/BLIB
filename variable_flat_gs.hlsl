#include "variable_flat_gs.hlsli"

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
	
    float2 size = gin[0].uv_size * gin[0].scale;
    size.y *= y_invert_c;
	
    float xflip = step(size.x, 0);
    float yflip = step(size.y, 0);
	float xflip_c = 1 - xflip * 2;
    float yflip_c = 1 - yflip * 2;
	
    //position.x += size.x * xflip;
    //position.y += size.y * yflip;
    size.x *= xflip_c;
    size.y *= yflip_c;
	    
    float2 corner[4] =
    {
        float2(0,      0),
        float2(size.x, 0),
        float2(0,      size.y),
        float2(size.x, size.y)
    };
    
    float2  pivot   = (gin[0].pivot + float2(1, 1)) * 0.5 * size;
    float   angle   = gin[0].rotation * y_invert_c;
    pivot.y *= y_invert_c;
    
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        corner[i] = position + pivot + rotate((corner[i] - pivot), angle);
        corner[i].x = 2.0f * corner[i].x / viewport.x - 1.0f;
        corner[i].y = 1.0f - 2.0f * corner[i].y / viewport.y;

        pins[i].position = float4(corner[i], 0, 1);
    }
	
    yflip = lerp(yflip, 1 - yflip, y_invert_b);
    
    float u[2] = { 0, 1 };
    float v[2] = { 0, 1 };
   
    pins[0].texcoord = ( float2( u[int(    xflip)], v[int(    yflip)] ) * gin[0].uv_size + gin[0].uv_position ) / texture_size;
	pins[1].texcoord = ( float2( u[int(1 - xflip)], v[int(    yflip)] ) * gin[0].uv_size + gin[0].uv_position ) / texture_size;
	pins[2].texcoord = ( float2( u[int(    xflip)], v[int(1 - yflip)] ) * gin[0].uv_size + gin[0].uv_position ) / texture_size;
	pins[3].texcoord = ( float2( u[int(1 - xflip)], v[int(1 - yflip)] ) * gin[0].uv_size + gin[0].uv_position ) / texture_size;
		
	[unroll]
    for (int j = 0; j < 4; j++)
    {
        output.Append(pins[j]);
    }
    output.RestartStrip();

}