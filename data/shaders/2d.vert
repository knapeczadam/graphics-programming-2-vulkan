#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec2 in_uv;
layout (location = 4) in vec3 in_tangent;

layout (location = 0) out vec3 out_color;

layout (set = 0, binding = 0) uniform global_ubo
{
    mat4 projection;
    mat4 view;
} ubo;

// There must be no more than one push constant block statically used per shader entry point.
layout (push_constant) uniform Push
{
    mat4 model_matrix;
} push;

void main()
{
    vec4 position_world = push.model_matrix * vec4(in_position, 1.0f);
    
    out_color           = in_color;
    gl_Position         = ubo.projection * (ubo.view * position_world);
}