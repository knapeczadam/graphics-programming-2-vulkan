#version 450

layout (location = 0) in vec2 frag_offset;
layout (location = 0) out vec4 out_color;

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

layout (push_constant) uniform Push
{
    vec4 position;
    vec4 color;
    float radius;
} push;

void main()
{
    float dis = sqrt(dot(frag_offset, frag_offset));
    if (dis >= 1.0f)
    {
        discard;
    }
    out_color = vec4(push.color.rgb, 1.0f);
}