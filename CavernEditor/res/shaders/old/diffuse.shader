#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 u_ModelMat;
layout(std140) uniform SceneData
{
    uniform mat4 u_ProjViewMat;
};


out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main()
{
    FragPos = vec3(u_ModelMat * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(u_ModelMat))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = u_ProjViewMat * vec4(FragPos, 1.0);
}

#shader fragment
#version 330 core

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    float specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 viewPos;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

layout(std140) uniform LightParams
{
    uniform Light light;
};

uniform Material material;

void main()
{
    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    // specular
    vec3 viewDir = normalize(light.viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;

    // attenuation
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}