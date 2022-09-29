precision highp float;
attribute vec3 position;
attribute vec3 tangent;
attribute vec3 normal;
attribute vec2 uv0;

uniform float time;
uniform mat4 projectionView;
uniform mat4 model;
uniform mat3 normalSpace;
uniform vec3 eyePosition;
uniform vec3 lightPos;

varying vec2 vTexCoord;

varying vec3 v_normal;
varying vec3 v_tangent;
varying vec3 v_toLight;
varying vec3 v_toView;

varying mat3 TBN;


void main()
{
//    const vec3 lightPosition = vec3(0., 10., 5.);
    v_normal = normalSpace * normal;
    v_tangent = normalSpace * tangent;

    vec3 T = normalize(vec3(model * vec4(v_tangent,   0.0)));
    vec3 N = normalize(vec3(model * vec4(v_normal,    0.0)));
    vec3 B = normalize(cross(N, T));
    TBN = mat3(T, B, N);

    gl_Position = projectionView * model * vec4(position , 1.0);

    v_toLight = lightPos - gl_Position.xyz;

    v_toView = eyePosition - gl_Position.xyz;
    vTexCoord = uv0;
}

