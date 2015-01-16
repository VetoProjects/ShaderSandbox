#version 330 core


// Values from the vertex shaders
in vec2 uv;
in vec3 wsPosition;
in vec3 csNormal;
in vec3 csEyeDirection;
in vec3 csLightDirection;


// Values that stay constant for the whole mesh.
uniform vec3 wsLightPosition;
uniform float time;
uniform vec2 mouse;
uniform float ration;

uniform sampler1D audioLeftData;
uniform sampler1D audioRightData;


// Ouput data
out vec4 color;


float left (float val){ return texture(audioLeftData , val).r ; }
float right(float val){ return texture(audioRightData, val).r ; }


void main(){
    // Light emission properties
    vec3 LightColor = vec3(1,1,1);
    float LightPower = 10.0f;


    // Material properties (Maybe texture)
    vec3 MaterialDiffuseColor = vec3(1.0, 0.2, 0.1);
    vec3 MaterialAmbientColor  = vec3(0.1, 0.1, 0.1) * MaterialDiffuseColor;
    vec3 MaterialSpecularColor = vec3(0.3, 0.3, 0.3);

    // Distance to the light
    float dist = distance(wsLightPosition, wsPosition);

    // Normal of the computed fragment, in camera space
    vec3 n = normalize(csNormal);
    // Direction of the light (from the fragment to the light)
    vec3 l = normalize(csLightDirection);
    // Cosine of the angle between the normal and the light direction,
    // clamped above 0
    //  - light is at the vertical of the triangle -> 1
    //  - light is perpendicular to the triangle -> 0
    //  - light is behind the triangle -> 0
    float cosTheta = clamp(dot(n, l), 0, 1);

    // Eye vector (towards the camera)
    vec3 E = normalize(csEyeDirection);
    // Direction in which the triangle reflects the light
    vec3 R = reflect(-l, n);
    // Cosine of the angle between the Eye vector and the Reflect vector,
    // clamped to 0
    //  - Looking into the reflection -> 1
    //  - Looking elsewhere -> < 1
    float cosAlpha = clamp(dot(E, R), 0, 1);

    color.rgb =
            // Ambient : simulates indirect lighting
            MaterialAmbientColor +
            // Diffuse : "color" of the object
            MaterialDiffuseColor * LightColor * LightPower * cosTheta / (dist * dist) +
            // Specular : reflective highlight, like a mirror
            MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha, 5) / (dist * dist);
    color.a = 1;
}
