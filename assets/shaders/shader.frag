#version 450

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in VS_OUT
{
    vec3 color;
    vec2 uv;
    vec3 normal;
} fs_in;

layout(binding = 1) uniform sampler2D texture_sampler;

layout(location = 0) out vec4 frag_color;

void main() 
{
    //frag_color = texture(texture_sampler, fs_in.uv);
    frag_color = vec4(fs_in.color * texture(texture_sampler, fs_in.uv).rgb, 1.0);
}