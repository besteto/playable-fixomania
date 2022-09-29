precision highp float;
uniform vec3 ambientDirection;
uniform vec3 lightPositions[10];
uniform float time;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
varying vec2 vTexCoord;

void main()
{
   vec4 texMain = texture2D(texture0, vTexCoord);
   vec4 texLight = texture2D(texture1, vTexCoord);
   vec4 texSpec = texture2D(texture2, vTexCoord);

   vec4 theColor = mix(texMain, texSpec+texLight, abs(sin(time))*.3);

   gl_FragColor = theColor;
}
