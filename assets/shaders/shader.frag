#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in VS_OUT
{
    vec3 color;
} fs_in;

layout(location = 0) out vec4 frag_color;

void main() 
{
    frag_color = vec4(fs_in.color, 1.0);
}