#version 450

layout (location = 0) in vec2 frag_offset;
layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform global_ubo
{
    mat4 projection;
    mat4 view;
    vec4 ambient_light_color; // w is intensity
    vec3 light_position;
    vec4 light_color;
} ubo;

void main()
{
    float dis = sqrt(dot(frag_offset, frag_offset));
    if (dis >= 1.0f)
    {
        discard;
    }
    out_color = vec4(ubo.light_color.rgb, 1.0f);
}