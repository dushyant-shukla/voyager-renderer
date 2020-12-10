#version 460 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


//layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 normal;
//layout(location = 2) in vec2 uv;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in ivec4 bone_ids;
layout(location = 6) in vec4 bone_weights;

layout(set = 0, binding = 0) uniform ViewUBO
{
    mat4 projection;
    mat4 view;
} view_ubo;

layout (push_constant) uniform PushModel
{
    mat4 model;
} push_model;

layout (location = 0) out VS_OUT
{
    vec2 uv;
    vec3 normal;
    vec3 frag_position;
} vs_out;

void main() 
{
    gl_Position = view_ubo.projection 
                  * view_ubo.view
                  * push_model.model
                  * vec4(position, 1.0);

    mat3 normal_matrix = mat3(transpose(inverse(push_model.model)));

    vs_out.uv = uv;
    vs_out.normal = normal_matrix * normal;
    vs_out.frag_position = vec3(push_model.model * vec4(position, 1.0));
}