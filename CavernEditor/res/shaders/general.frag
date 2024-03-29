#version 460 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int  DrawID;

const int MAX_LIGHTS_COUNT = 16;
const int MAX_DIRNSPOT_LIGHTS = 8;
in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;

    //for objects with normal map
    vec3 TangentLightPos[MAX_LIGHTS_COUNT];
    vec3 TangentViewPos;
    vec3 TangentFragPos;

    //for dir. light shadow mapping
    vec4 FragPosLightSpace[MAX_DIRNSPOT_LIGHTS];
} fs_in;

#include "defs.glsl"

uniform uint u_ObjType;
uniform int u_DrawId;

uniform Material material;

#include "shadowMapping.glsl"

void main()
{
    vec3 Lighting = vec3(0.0, 0.0, 0.0);

    vec3 lighting = vec3(0.0, 0.0, 0.0);


    int j = 0;
    for (int i = 0; i < sceneData.lightsCount; ++i)
    {
        Light light = lightData.lights[i];
        //if (!light.enabled)
        //    continue;
        
        vec3 normal;
        vec3 viewDir;
        vec3 lightDir;

        vec3 matSpec;

        if (u_ObjType == DIFF_ONLY || u_ObjType == DIFF_N_SPEC)
        {
            normal = normalize(fs_in.Normal);
            viewDir = normalize(sceneData.viewPos - fs_in.FragPos);
            //if (light.type != POINT_LIGHT)
            //    lightDir = normalize(-light.direction);
            //else
            lightDir = normalize(light.position - fs_in.FragPos);
        }
        else if (u_ObjType == DIFF_N_NORMAL)
        {
            // obtain normal from normal map in range [0,1]
            normal = texture(material.normalTex, fs_in.TexCoords).rgb;
            // transform normal vector to range [-1,1]
            normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
            lightDir = normalize(fs_in.TangentLightPos[i] - fs_in.TangentFragPos);
            viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
        }
        
        vec3 halfwayDir = normalize(lightDir + viewDir);

        float diff = max(dot(normal, lightDir), 0.0);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

        // combine results
        vec3 color = texture(material.diffuseTex, fs_in.TexCoords).rgb;

        vec3 ambient  = light.ambient * color;
        vec3 diffuse  = light.diffuse * diff * color;
        vec3 specular = light.specular * spec * matSpec;

        ambient  *= light.color * light.brightness;
        diffuse  *= light.color * light.brightness;
        specular *= light.color * light.brightness;
        
        // spotlight (soft edges)
        if (light.type == SPOT_LIGHT)
        {
            //In case of normal maps can't use lightDir because it's in tangent space
            vec3 wlightDir = normalize(light.position - fs_in.FragPos);
    
            float theta = dot(wlightDir, normalize(-light.direction)); 
            float epsilon = (light.cutOff - light.outerCutOff);
            float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
            diffuse  *= intensity;
            specular *= intensity;
        }
        // attenuation
        if (light.type != DIRECTIONAL_LIGHT)
        {
            float distance = length(light.position - fs_in.FragPos);
            float attenuation = 1.0 /
                (light.constant + light.linear * distance + light.quadratic * (distance * distance));

            ambient *= attenuation;
            diffuse *= attenuation;
            specular *= attenuation;
        }

        float shadow = 0.0;
        if (sceneData.castShadows)
        {
            switch (light.type)
            {
            case SPOT_LIGHT:
                shadow = SpotShadowCalc(fs_in.Normal, fs_in.FragPos, 
                    fs_in.FragPosLightSpace[j], light.position, light.atlasoffset, light.mipmaplevel);
                ++j;
                break;
            case DIRECTIONAL_LIGHT:
                shadow = DirShadowCalc(fs_in.Normal, fs_in.FragPos, 
                    fs_in.FragPosLightSpace[j], light.position, light.atlasoffset);
                ++j;
                break;
            case POINT_LIGHT:
                shadow = PointShadowCalc(fs_in.FragPos, light.position, light.atlasoffset, light.mipmaplevel);
                break;
            }
        }
        lighting = (ambient + (1.0 - shadow) * (diffuse + specular));
        Lighting += lighting;
    }
    //Shadow = clamp(Shadow, 0.f, 1.f);
    //Lighting = (Ambient + (1.0 - Shadow) * (Diffuse + Specular));

    FragColor = vec4(Lighting, 1.0);
    DrawID = u_DrawId;
}
