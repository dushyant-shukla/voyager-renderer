# version 460 core

layout(location = 0) in vec4 position;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 projection;
    mat4 view;
} ubo;

layout (push_constant) uniform PushModel
{
    mat4 model;
    int render_joints; // 0 - joints, 1 - lines
} push_model;

layout (location = 0) out VS_OUT
{
    int render_joints;
} vs_out;

void main()
{
   gl_PointSize = 5.0;
   vs_out.render_joints = push_model.render_joints;
   gl_Position = ubo.projection 
                    * ubo.view
                    * push_model.model
                    * position;
} 
