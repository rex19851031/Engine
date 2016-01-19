#version 130
in vec2 v_textureCoords;
in vec4 v_color;
in vec4 v_position;

uniform sampler2D u_diffuseMap;

out vec4 fragment_color;

void main(void)
{
	vec4 diffuseColor = texture2D(u_diffuseMap,v_textureCoords);
	fragment_color = v_color * diffuseColor;
}