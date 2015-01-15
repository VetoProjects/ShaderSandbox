#version 330 core

// ms: Model Space
// ws: World Space
// cs: Camera Space

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 msVertexPosition;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 msVertexNormal;

// Output data ; will be set for each fragment.
out vec2 uv;
out vec3 wsPosition;
out vec3 csNormal;
out vec3 csEyeDirection;
out vec3 csLightDirection;

// Values that stay constant for the whole mesh.
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 MV;
uniform mat4 MVP;
uniform vec3 wsLightPosition;

uniform float time;
uniform vec2 mouse;
uniform float ration;

uniform sampler1D audioLeftData;
uniform sampler1D audioRightData;


float left (float val){ return texture(audioLeftData , val).r ; }
float right(float val){ return texture(audioRightData, val).r ; }

void main(){
        // Prepare vectors for multiplication
        vec4 msVertPos  = vec4(msVertexPosition, 1);
        vec4 wsLightPos = vec4(wsLightPosition, 1);

        // Output position of the vertex, in clip space : MVP * position
        float t = time / 1000;
        float st = sin(t);
        float ct = cos(t);
        mat4 turn = mat4(
            st,  ct, 0, 0,
            ct, -st, 0, 0,
            0 , 0  , 1, 0,
            0 , 0  , 0, 1
        );
        gl_Position =  MVP * turn * msVertPos * 0.005;

        // Position of the vertex, in worldspace : M * position
        wsPosition = (M * msVertPos).xyz;

        // Vector that goes from the vertex to the camera, in camera space.
        // In camera space, the camera is at the origin (0,0,0).
        vec3 csVertexPosition = (MV * msVertPos).xyz;
        csEyeDirection = vec3(0,0,0) - csVertexPosition.xyz;

        // Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
        vec3 csLightPosition = (V * wsLightPos).xyz ;
        csLightDirection = csLightPosition + csEyeDirection;

        // Normal of the the vertex, in camera space
        csNormal = (MV * vec4(msVertexNormal, 0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.

        // UV of the vertex. No special space for this one.
        uv = vertexUV;
}
