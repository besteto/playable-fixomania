precision highp float;
uniform vec3 ambientDirection;
uniform vec3 lightPositions[10];

uniform sampler2D texture0;
uniform vec4 color;
uniform samplerCube cubemap0;
varying vec2 vTexCoord;
uniform float reflectCoef;
uniform float time;

varying vec3 v_normal;
varying vec3 v_tangent;
varying vec3 v_toLight;
varying vec3 v_toView;

void main()
{
   const vec4 specColor = vec4 ( 1.0, 1.0, 1.0, 1.0 );
   const float specPower = 1.0;

   vec4 czar = texture2D(texture0, vTexCoord);
//   vec4 diffColor = vec4(color.rgb + czar.rgb, 1.);
   vec4 diffColor = color;
   diffColor += vec4 ( .5, 0.0, 0.0, 1.0 ) * abs(sin(time));

   vec3 n2 = normalize ( v_normal);
   vec3 v2 = normalize ( v_toView );
   vec3 r  = reflect   ( -v2, n2 );

   vec3 l2 = normalize ( v_toLight );

   vec4 diff1 = diffColor * max ( dot ( n2, l2 ), 0.0 );
   vec4 spec1 = specColor * pow ( max ( dot ( l2, r ), 0.0 ), specPower );
   spec1 = spec1 * (1. - czar.r);
   vec4 reflectColor = textureCube(cubemap0, r) * (1. - czar.r) * reflectCoef;
   vec4 thePulse = vec4 ( .5, 0.0, 0.0, 1.0 ) * abs(sin(time)*0.5);

   vec4 theColor = diff1 + spec1 + reflectColor + thePulse;

   gl_FragColor = theColor;

}
