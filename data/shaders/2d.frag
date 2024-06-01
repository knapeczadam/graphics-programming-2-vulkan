#version 450

layout (location = 0) in vec3 in_color;
layout (location = 3) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 5) uniform sampler2D tex;

layout (push_constant) uniform Push
{
    mat4 model_matrix;
    bool use_texture;
} push;

void main()
{
    if (push.use_texture)
    {
        out_color.rgb = texture(tex, in_uv).rgb;
        out_color.a = 1.0f;
        return;
    }
    out_color = vec4(in_color, 1.0f);
}