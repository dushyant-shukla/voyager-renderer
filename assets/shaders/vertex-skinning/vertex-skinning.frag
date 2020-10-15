#version 460 core

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in VS_OUT
{
    vec3 color;
    vec2 uv;
    vec3 normal;
} fs_in;

layout(set = 1, binding = 0) uniform sampler2D texture_sampler;
layout(set = 1, binding = 1) uniform sampler2D texture_sampler1;
layout(set = 1, binding = 2) uniform sampler2D texture_sampler2;

layout(location = 0) out vec4 frag_color;

void main() 
{
    //frag_color = texture(texture_sampler, fs_in.uv);
    frag_color = texture(texture_sampler, fs_in.uv);
    //frag_color = vec4(fs_in.color, 1.0);
    //if (frag_color.w < 0.6) { discard; }
}