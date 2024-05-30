#version 450

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec3 in_position;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec2 in_uv;
layout (location = 4) in vec3 in_tangent;

layout (location = 0) out vec4 out_color;

struct light
{
    vec4 position; // w : 0.0f for directional light, 1.0f for point light
    vec4 color;    // w : intensity
};

layout (set = 0, binding = 0) uniform global_ubo
{
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
    vec4 ambient_light_color; // w is intensity
    light lights[10];
    int num_lights;
} ubo;

layout (set = 0, binding = 1) uniform sampler2D diffuse_texture;
layout (set = 0, binding = 2) uniform sampler2D normal_texture;
layout (set = 0, binding = 3) uniform sampler2D specular_texture;
layout (set = 0, binding = 4) uniform sampler2D gloss_texture;

#define PI 3.1415926535897932384626433832795
#define DEG_TO_RAD 0.01745329251994329576923690768489

const vec3 dielectric = vec3(0.04f);
const float ambient = 0.01f;


layout (push_constant) uniform Push
{
    mat4 model_matrix;
    mat4 normal_matrix;
    float r;
    float g;
    float b;
    float metallic;
    float roughness;
} push;

/*
    * light          : light source
    * point_to_shade : target position
    * return         : light intensity - toal amount of energy the light emits (~radiant intensity)
*/
vec3 radiance(light light, vec3 point_to_shade)
{
    if (light.position.w == 0.0f)
    {
        return light.color.rgb * light.color.w;
    }
    vec3 dir_to_light = light.position.xyz - point_to_shade;
    float attenuation = 1.0f / dot(dir_to_light, dir_to_light);
    return light.color.rgb * light.color.w * attenuation;
}

/*
    * cd : diffuse color (color of the material)
    * kd : diffuse reflectio coefficient: how reflective is this matee material, should range [0, 1]
*/
vec3 lambert(vec3 cd, vec3 kd)
{
    /*
        * rho: reflectivity of the surface, or to be more accurate, the perfect diffuse reflectance
    */
    vec3 rho = cd * kd;
    return rho / PI;
}

/*
    * n_dot_h : dot product between normal and half vector
    * a       : roughness value squared
*/
float trowbridge_reitz_ggx(float n_dot_h, float a)
{
    float a2  = a * a;
    float num = a2;
    
    float n_dot_h2 = n_dot_h * n_dot_h;
    float denom    = n_dot_h2 * (a2 - 1.0f) + 1.0f;
    
    return num / (PI * denom * denom);
}

/*
    * h_dot_v : dot product between half vector and view direction
    * f0      : base reflectivity of a surface on IOR (Indices Of Refrection), this is different for dielectrics (non-metals) and conductors (metals)
*/
vec3 fresnel_schlick(float h_dot_v, vec3 f0)
{
    return f0 + (1.0f - f0) * pow(1.0f - h_dot_v, 5.0f);
}

/*
    * n_dot_v : dot product between normal and view direction
    * n_dot_l : dot product between normal and light direction
    * a       : roughness value squared
*/
float geometry_schlick_smith_ggx(float n_dot_v, float n_dot_l, float a)
{
    /*
        * k_direct:a (roughness squared) remapped based on whether you use the function with direction or indirect lighting
    */  
    float num = a + 1.0f;
    float num2 = num * num;
    float k_direct = num2 / 8.0f;
    
    float g1 = n_dot_v / (n_dot_v * (1.0f - k_direct) + k_direct);
    float g2 = n_dot_l / (n_dot_l * (1.0f - k_direct) + k_direct);
    return g1 * g2;
}

/*
    * n         : surface normal
    * l         : normalized light direction
    * v         : normalized view direction
    * albedo    : color of the material
    * metallic  : 0.0f for dielectrics (non-metals) and 1.0f for conductors (metals)
    * roughness : roughness of the material
    * return    : specular reflectance
*/
vec3 brdf(vec3 n, vec3 l, vec3 v, vec3 albedo, float metallic, float roughness)
{
    /*
        * f0: base reflectivity of a surface on IOR (Indices Of Refrection), this is different for dielectrics (non-metals) and conductors (metals)
        * h : half vector between light direction and view direction
        * a : roughness value squared
    */
    vec3 f0 = metallic == 0.0f ? dielectric : albedo;
    vec3 h  = normalize(l + v);
    float a = roughness * roughness;
    
    float n_dot_h = clamp(dot(n, h), 0.0f, 1.0f);
    float n_dot_l = clamp(dot(n, l), 0.0f, 1.0f);
    float n_dot_v = clamp(dot(n, v), 0.0f, 1.0f);
    float h_dot_v = clamp(dot(h, v), 0.0f, 1.0f);
    
    
    /*
        * d     : normal distribution function: how many microfacets point in right direction?
        * f     : fresnel function: how reflective are the microfacets?
        * g     : geometry function: how much shadowing and masking is happening because of the microfacets?
        * denom : reprojection of the factors
    */  
    float d = trowbridge_reitz_ggx(n_dot_h, a);
    vec3 f  = fresnel_schlick(h_dot_v, f0);
    float g = geometry_schlick_smith_ggx(n_dot_v, n_dot_l, a);
    float denom = 4.0f * n_dot_v * n_dot_l;
        
    return d * f * g / denom;
}

bool gUseNormalMap = true;
vec3 gLightDir = vec3(0.577f, 0.577f, 0.577f);
float gLightIntensity = 1.0f;
float gKD = 7.0f;
float gShininess = 25.0f;
vec3 gAmbientColor = vec3(0.03f);
int gShadingMode = 3;

vec4 shade_pixel(vec3 normal, vec3 tangent, vec3 view_dir, vec3 diffuse_color, vec3 normal_color, vec3 specular_color, float gloss) 
{
    vec3 color = vec3(0);

    // Binormal
    vec3 binormal = cross(normal, tangent);

    // Tangent-space transformation matrix
    mat3 tangent_space = mat3(tangent, binormal, normal);

    // Remap normal from [0, 1] to [-1, 1]
    normal_color = normal_color * 2.0f - vec3(1.0f);

    // Transform normal from tangent-space to world-space
    normal = gUseNormalMap ? tangent_space * normal_color : normal;

    // Light direction
    vec3 light_dir = normalize(gLightDir);

    // Radiance (directional light)
    vec3 radiance = (vec3(1.0f, 1.0f, 1.0f) * gLightIntensity);

    // Observed area
    vec3 observed_area = vec3(clamp(dot(normal, -light_dir), 0.0f, 1.0f));

    // Diffuse lighting
    vec3 diffuse = diffuse_color * gKD / PI;

    // Phong specular lighting
    vec3 reflected_light = reflect(-light_dir, normal);
    float cos_alpha = clamp(dot(reflected_light, -view_dir), 0.0f, 1.0f);
    vec3 phong = specular_color * pow(cos_alpha, gloss * gShininess);

    if (gShadingMode == 0)
    {
        color = observed_area;
    }
    else if (gShadingMode == 1)
    {
        color = diffuse * observed_area;
    }
    else if (gShadingMode == 2)
    {
        color = phong * observed_area;
    }
    else
    {
        color = radiance * (diffuse + phong + gAmbientColor) * observed_area;
    }
    return vec4(color, 1.0);
}


void main()
{
    vec3 camera_pos_world = ubo.inverse_view[3].xyz;
    vec3 view_dir         = normalize(camera_pos_world - in_position);

    vec3 base_color = vec3(push.r, push.g, push.b);
    float metallic  = push.metallic;
    float roughness = push.roughness;

//    out_color.rgb = base_color * ambient;
    out_color.a   = 1.0f;
    
    vec3 diffuse_color = texture(diffuse_texture, in_uv).rgb;
    vec3 normal_color  = texture(normal_texture, in_uv).rgb;
    vec3 specular_color = texture(specular_texture, in_uv).rgb;
    float gloss_color = texture(gloss_texture, in_uv).r;
    
    out_color = shade_pixel(in_normal, in_tangent, view_dir, diffuse_color, normal_color, specular_color, gloss_color);
    /*
    
    for (int i = 0; i < ubo.num_lights; ++i)
    {
        light light         = ubo.lights[i];
        vec3 l              = normalize(light.position.xyz - in_position);
        float observed_area = clamp(dot(in_normal, l), 0.0f, 1.0f);
        
        if (observed_area > 0.0f)
        {
            vec3 specular = brdf(in_normal, l, view_dir, base_color, metallic, roughness);
            
            vec3 kd      = metallic == 0.0f ? 1.0f - specular : vec3(0.0f);
            vec3 diffuse = lambert(base_color, kd);

            out_color.rgb +=  radiance(light, in_position) * (diffuse + specular) * observed_area;
        }
    }
*/
}
