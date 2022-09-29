precision highp float;
uniform vec3 ambientDirection;
uniform vec3 lightPositions[10];

uniform sampler2D texture0;
uniform sampler2D texture1;
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
   const float specPower = 40.0;

   vec4 diffColor = texture2D(texture0, vTexCoord);
   vec4 specColor = texture2D(texture1, vTexCoord);

   vec3 n2 = normalize ( v_normal );
   vec3 v2 = normalize ( v_toView );
   vec3 r  = reflect   ( -v2, n2 );

   vec3 l2 = normalize ( v_toLight );

   vec4 spec = specColor * pow ( max ( dot ( l2, r ), 0.0 ), specPower );
   vec4 reflectColor = textureCube(cubemap0, r) * reflectCoef * specColor;

   vec4 theColor = diffColor + spec + reflectColor;

   gl_FragColor = theColor;

}
