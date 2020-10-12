#version 460 core

#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec4 bone_ids;
layout(location = 6) in vec4 bone_weights;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 view_projection;
    vec3 view_position;
} vp_buffer;

layout (location = 0) out VS_OUT
{
    vec3 color;
    vec2 uv;
    vec3 normal;
} vs_out;

layout (push_constant) uniform PushModel
{
    mat4 model;
} push_model;

void main() 
{
    gl_Position = vp_buffer.view_projection * push_model.model * vec4(position, 1.0);
    vs_out.color = color;
    vs_out.uv = uv;
    vs_out.normal = normal;
}