#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec2 in_uv;
layout (location = 4) in vec3 in_tangent;

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec3 out_position;
layout (location = 2) out vec3 out_normal;
layout (location = 3) out vec2 out_uv;
layout (location = 4) out vec3 out_tangent;

struct point_light
{
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout (set = 0, binding = 0) uniform global_ubo
{
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
    vec4 ambient_light_color; // w is intensity
    point_light point_lights[10];
    int num_lights;
} ubo;

// There must be no more than one push constant block statically used per shader entry point.
layout (push_constant) uniform Push
{
    mat4 model_matrix;
    mat4 normal_matrix;
    int  shading_mode;
    bool use_normal_map;
} push;

void main()
{
    vec4 position_world = push.model_matrix * vec4(in_position, 1.0f);
    gl_Position         = ubo.projection * (ubo.view * position_world);

    out_normal   = normalize(mat3(push.normal_matrix) * in_normal);
    out_tangent  = normalize(mat3(push.normal_matrix) * in_tangent.xyz);
    out_position = position_world.xyz;
    out_color    = in_color;
    out_uv       = in_uv;
}