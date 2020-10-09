#version 460 core

#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp_buffer;

layout (location = 0) out VS_OUT
{
    vec3 color;
    vec2 uv;
    vec3 normal;
} vs_out;

void main() 
{
    gl_Position = mvp_buffer.projection * mvp_buffer.view * mvp_buffer.model * vec4(position, 1.0);
    vs_out.color = color;
    vs_out.uv = uv;
    vs_out.normal = normal;
}