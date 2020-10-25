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
} push_model;

void main()
{
   gl_PointSize = 5.0;
   gl_Position = ubo.projection 
                    * ubo.view
                    * push_model.model
                    * position;
} 
