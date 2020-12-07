# version 460 core

layout(location = 0) in vec4 position;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 projection;
    mat4 view;
} ubo;

layout (push_constant) uniform PushModel
{
    mat4 joint_model;
    mat4 line_model;
    int render_joints; // 0 - joints, 1 - lines
} push_model;

void main()
{
   gl_PointSize = 5.0;
   mat4 model = mat4(1.0);
   if(push_model.render_joints == 0)
   {
        model = push_model.joint_model;
   }
   else
   {
        model = push_model.joint_model;
   }
   gl_Position = ubo.projection 
                    * ubo.view
                    * model
                    * position;
} 
