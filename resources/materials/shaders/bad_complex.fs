precision highp float;
#define PI 3.14159265358979323846

uniform vec3 ambientDirection;
uniform vec3 lightPositions[10];
uniform float time;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform vec4 color;
uniform samplerCube cubemap0;

uniform float specSwitch;
uniform float emissiveSwitch;

varying vec2 vTexCoord;

varying vec3 v_normal;
varying vec3 v_tangent;
varying vec3 v_toLight;
varying vec3 v_toView;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
   float a      = roughness*roughness;
   float a2     = a*a;
   float NdotH  = max(dot(N, H), 0.0);
   float NdotH2 = NdotH*NdotH;

   float num   = a2;
   float denom = (NdotH2 * (a2 - 1.0) + 1.0);
   denom = PI * denom * denom;

   return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
   float r = (roughness + 1.0);
   float k = (r*r) / 8.0;

   float num   = NdotV;
   float denom = NdotV * (1.0 - k) + k;

   return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
   float NdotV = max(dot(N, V), 0.0);
   float NdotL = max(dot(N, L), 0.0);
   float ggx2  = GeometrySchlickGGX(NdotV, roughness);
   float ggx1  = GeometrySchlickGGX(NdotL, roughness);

   return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
   return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
   const vec4  specColor = vec4 ( 1.0, 1.0, 1.0, 1.0 );
   const float specPower = 1.0;

   vec4 diffuseC = texture2D(texture0, vTexCoord);
   vec4 specularC = texture2D(texture1, vTexCoord) + specColor/2.;
//   vec4 opacityC = texture2D(texture2, vTexCoord);
   vec4 roughC = texture2D(texture3, vTexCoord);
//   vec4 glossC = texture2D(texture4, vTexCoord);
//   vec4 emissiveC = texture2D(texture5, vTexCoord);

   vec3 n2 = normalize ( v_normal );
   vec3 v2 = normalize ( v_toView );
   vec3 l2 = normalize ( v_toLight );

   vec3 h = normalize(v2 + l2);
   vec3 r  = reflect   ( -v2, n2 );


   vec4 diff1 = diffuseC * max ( dot ( n2, l2 ), 0.0 );
//   vec4 spec1 = specularC * pow ( max ( dot ( l2, r ), 0.0 ), specPower );

   vec3 F0 = vec3(0.04);
   float roughness = roughC.r;

   float NDF = DistributionGGX(n2, h, roughness);
   float G   = GeometrySmith(n2, v2, l2, roughness);
   vec3 F    = fresnelSchlick(max(dot(h, v2), 0.0), F0);

   vec3 numerator    = NDF * G * F;
   float denominator = 4.0 * max(dot(n2, v2), 0.0) * max(dot(n2, l2), 0.0);
   vec3 specular     = numerator / max(denominator, 0.001);

//   vec4 theColor = diff1;
//   theColor += spec1;
//
//   gl_FragColor = vec4(theColor.rgb, 1.);
   vec4 reflectColor = textureCube(cubemap0, r) *  (.5 - roughness);

   vec4 theColor = diff1 + reflectColor * 0.4;
   theColor.rgb += specular;

   vec4 thePulse = vec4 ( .5, 0.0, 0.0, 1.0 ) * abs(sin(time)*0.5);

   gl_FragColor = theColor + thePulse;

}
