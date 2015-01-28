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
out vec3 wsLightPosition;
out vec3 csNormal;
out vec3 csEyeDirection;
out vec3 csLightDirection;

// Values that stay constant for the whole mesh.
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

uniform float time;
uniform vec2 mouse;
uniform float ration;

uniform sampler1D audioLeftData;
uniform sampler1D audioRightData;


void main(){
    vec4 wsLightPos = vec4(0,2,2, 1);

    // manipulation of MVP before using it
    mat4 m = M, v = V, p = P, mv = v * m, mvp = p * mv;

    // Prepare vectors for multiplication
    vec4 msVertPos  = vec4(msVertexPosition, 1);

    // Output position of the vertex, in clip space : MVP * position
    gl_Position = mvp * msVertPos;

    // Position of the vertex, in worldspace : M * position
    wsPosition = (m * msVertPos).xyz;
    wsLightPosition = wsLightPos.xyz;

    // Vector that goes from the vertex to the camera, in camera space.
    // In camera space, the camera is at the origin (0,0,0).
    vec3 csVertexPosition = (mv * msVertPos).xyz;
    csEyeDirection = vec3(0,0,0) - csVertexPosition.xyz;

    // Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
    vec3 csLightPosition = (v * wsLightPos).xyz ;
    csLightDirection = csLightPosition + csEyeDirection;

    // Normal of the the vertex, in camera space
    csNormal = (mv * vec4(msVertexNormal, 0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.

    // UV of the vertex. No special space for this one.
    uv = vertexUV;
}



// Helper functions

float left (float val){ return texture(audioLeftData , val).r ; }
float right(float val){ return texture(audioRightData, val).r ; }

mat4 translate(float x, float y, float z){ return mat4(
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    x,y,z,1
); }

mat4 scale(float x, float y, float z){ return  mat4(
    x,0,0,0,
    0,y,0,0,
    0,0,z,0,
    0,0,0,1
); }

mat4 scale(float s){ return scale(s,s,s); }

mat4 rotateX(float a){ return mat4(
    1, 0     , 0     , 0,
    0, cos(a), sin(a), 0,
    0,-sin(a), cos(a), 0,
    0, 0     , 0     , 1

); }

mat4 rotateY(float a){ return mat4(
     cos(a), 0, sin(a), 0,
     0     , 1, 0     , 0,
    -sin(a), 0, cos(a), 0,
     0     , 0, 0     , 1

); }

mat4 rotateZ(float a){ return mat4(
     cos(a), sin(a), 0, 0,
    -sin(a), cos(a), 0, 0,
     0     , 0     , 1, 0,
     0     , 0     , 0, 1

); }
