#version 150 core

/*
 * 0xF x texture
 * 0xF0 y texture
 * 0x100 x offset
 * 0x200 y offset
 */
in int specifications;
/*
 * 0xFF000000 Alpha
 * 0x00FF0000 Red
 * 0x0000FF00 Green
 * 0x000000FF Blue
 */
in int color;
in ivec2 position;

uniform int win_width;
uniform int win_height;

out vec2 texCoords;
out vec4 Color;

// half pixel correction to get correct location of texel
// equal to 0.5 / texSize, in this case texture is 128x65 so 0.5 / 128 x 0.5 / 64
const float half_pxl_h = 0.00390625;
const float half_pxl_v = 0.0078125;

void main()
{
	gl_Position = vec4((2.0 * position.x) / win_width - 1.0, -((2.0 * position.y) / win_height - 1.0), 0.0, 1.0);
	texCoords = vec2((specifications & 0xF) / 16.0f, ((specifications >> 4) & 0xF) / 8.0f);
	texCoords += vec2(((specifications & 0x100) == 0x100) ? 0.0625f - half_pxl_h : half_pxl_h,
				((specifications & 0x200) == 0x200) ? 0.125f - half_pxl_v : half_pxl_v);
	Color = vec4((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, (color >> 24) & 0xFF);
	Color /= vec4(255.0f);
}