precision highp float;
uniform vec3 ambientDirection;
uniform vec3 lightPositions[10];
uniform float time;

uniform vec4 color;
uniform float reflectCoef;
uniform samplerCube cubemap0;

varying vec2 vTexCoord;

varying vec3 v_normal;
varying vec3 v_tangent;
varying vec3 v_toLight;
varying vec3 v_toView;

void main()
{
   const vec4 specularC = vec4 ( 1.0, 1.0, 1.0, 1.0 );
   const float specPower = 5.0;

   vec4 diffuseC = color;

   vec3 n2 = normalize ( v_normal );
   vec3 v2 = normalize ( v_toView );

   vec3 r  = reflect   ( -v2, n2 );

   vec3 l2 = normalize ( v_toLight );

   vec4 diff1 = diffuseC * max ( dot ( n2, l2 ), 0.0 );
   vec4 spec1 = specularC * pow ( max ( dot ( l2, r ), 0.0 ), specPower );
   vec4 reflectColor = textureCube(cubemap0, r) * reflectCoef;

   vec4 theColor = diff1 + spec1 + reflectColor;
   vec4 thePulse = vec4 ( .5, 0.0, 0.0, 1.0 ) * abs(sin(time)*0.5);

   gl_FragColor = theColor + thePulse;

}
