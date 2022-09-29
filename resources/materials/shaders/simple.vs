precision highp float;
attribute vec3 position;
attribute vec2 uv0;

uniform mat4 projectionView;
uniform mat4 model;
varying vec2 vTexCoord;

void main()
{
    gl_Position = projectionView * model * vec4(position , 1.0);
    vTexCoord = uv0;
}

