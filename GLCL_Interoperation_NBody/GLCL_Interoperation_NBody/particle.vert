#version 150

in vec2 vs_in_pos;
uniform mat4 M = mat4(1.0f);

out Vertex
{
	vec4 color;
} vertex;
 
void main()
{
	gl_Position = M * vec4(vs_in_pos, 0, 1);
	vertex.color = vec4(0,1,1,1);
}