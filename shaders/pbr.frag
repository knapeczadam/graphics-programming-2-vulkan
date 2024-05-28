#version 450

#define PI 3.1415926535897932384626433832795
#define DEG_TO_RAD 0.01745329251994329576923690768489

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 frag_pos_world;
layout (location = 2) in vec3 frag_normal_world;
layout (location = 3) in vec2 frag_uv;
layout (location = 4) in vec3 frag_tangent_world;

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

layout (set = 0, binding = 1) uniform sampler2D diffuse_texture;
layout (set = 0, binding = 2) uniform sampler2D normal_texture;
layout (set = 0, binding = 3) uniform sampler2D specular_texture;
layout (set = 0, binding = 4) uniform sampler2D gloss_texture;

layout (push_constant) uniform Push
{
    mat4 model_matrix;
    mat4 normal_matrix;
} push;

mat3 TBN;

vec4 ShadePixel(vec3 normal, vec3 tangent, vec3 viewDir, vec3 diffuseColor, vec3 normalColor, vec3 specularColor, float gloss) {
    bool gUseNormalMap = true;
    vec3 gLightDir = vec3(0.577f, -0.577f, 0.577f);
    float gLightIntensity = 1.0f;
    float gKD = 7.0f;
    float gShininess = 25.0f;
    vec3 gAmbientColor = vec3(0.3f);
    int gShadingMode = 3;

    vec3 color = vec3(0);

    tangent = (tangent - dot(tangent, normal)) * normal;
    vec3 bitangent = cross(normal, tangent);
    TBN = mat3(tangent, bitangent, normal);

    vec3 local_normal = 2.0 * texture(normal_texture, frag_uv).rgb - 1.0;
    vec3 normal_color = normalize(TBN * local_normal);

    //    // Binormal
    //    vec3 binormal = cross(normal, tangent);
    //    
    //    // Tangent-space transformation matrix
    //    mat3 tangentSpace = mat3(tangent, binormal, normal);
    //    
    //    // Remap normal from [0, 1] to [-1, 1]
    //    normalColor = normalColor * 2.0f - vec3(1.0f);
    //    
    //    // Transform normal from tangent-space to world-space
    normal = gUseNormalMap ? normal_color : normal;

    // Light direction
    vec3 lightDir = normalize(gLightDir);

    // Radiance (directional light)
    vec3 radiance = (vec3(1.0f, 1.0f, 1.0f) * gLightIntensity);

    // Observed area
    vec3 observedArea = vec3(clamp(dot(normal, (-lightDir)), 0.0, 1.0));

    // Diffuse lighting
    vec3 diffuse = diffuseColor * gKD / PI;

    // Phong specular lighting
    vec3 reflectedLight = reflect(-lightDir, normal);
    float cosAlpha = clamp(dot(reflectedLight, -viewDir), 0.0f, 1.0f);
    vec3 phong = specularColor * pow(cosAlpha, gloss * gShininess);

    if (gShadingMode == 0)
    {
        color = observedArea;
    }
    else if (gShadingMode == 1)
    {
        color = diffuse * observedArea;
    }
    else if (gShadingMode == 2)
    {
        color = phong * observedArea;
    }
    else
    {
        color = radiance * (diffuse + phong + gAmbientColor) * observedArea;
    }
    return vec4(color, 1.0);
}


void main()
{
    vec3 diffuse_light = ubo.ambient_light_color.rgb * ubo.ambient_light_color.w;
    vec3 specular_light = vec3(0.0f);
    vec3 surface_normal = normalize(frag_normal_world);

    vec3 camera_pos_world = ubo.inverse_view[3].xyz;
    vec3 view_dir = normalize(camera_pos_world - frag_pos_world);

/*
    for (int i = 0; i < ubo.num_lights; ++i)
    {
        point_light light = ubo.point_lights[i];
        vec3 direction_to_light = light.position.xyz - frag_pos_world;
        float attenuation = 1.0f / dot(direction_to_light, direction_to_light); // distance squared
        direction_to_light = normalize(direction_to_light);
        
        float cos_angle_incidence = max(dot(surface_normal, direction_to_light), 0.0f);
        vec3 intensity = light.color.rgb * light.color.w * attenuation;
        
        diffuse_light += intensity * cos_angle_incidence;
        
        vec3 half_angle = normalize(direction_to_light + view_dir);
        float blinn_term = dot(surface_normal, half_angle);
        blinn_term = clamp(blinn_term, 0.0f, 1.0f);
        blinn_term = pow(blinn_term, 512.0f);
        
        specular_light += intensity * blinn_term;
    }
    */

    //    vec3 image_color = texture(diffuse_texture, frag_uv).rgb;
    //    out_color = vec4((diffuse_light * frag_color + specular_light * frag_color) * image_color, 1.0f);

    vec3 normal = frag_normal_world;
    vec3 tangent = frag_tangent_world.xyz;
    tangent = (tangent - dot(tangent, normal)) * normal;
    vec3 bitangent = cross(normal, tangent);
    TBN = mat3(tangent, bitangent, normal);

    vec3 local_normal = 2.0 * texture(normal_texture, frag_uv).rgb - 1.0;
    normal = normalize(TBN * local_normal);



    vec3 image_color = texture(diffuse_texture, frag_uv).rgb;
    vec3 normal_color = texture(normal_texture, frag_uv).rgb;
    vec3 specular_color = texture(specular_texture, frag_uv).rgb;
    float gloss = texture(gloss_texture, frag_uv).r;
    out_color = ShadePixel(frag_normal_world, frag_tangent_world, view_dir, image_color, normal_color, specular_color, gloss);
    //    out_color = vec4(texture(normal_texture, frag_uv).rgb, 1.0);
}
