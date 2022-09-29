precision highp float;
uniform vec3 ambientDirection;
uniform vec3 lightPositions[10];

uniform sampler2D texture0;
uniform vec4 color;
uniform float opacity;
uniform float time;
varying vec2 vTexCoord;

varying vec3 v_normal;
varying vec3 v_tangent;
varying vec3 v_toLight;
varying vec3 v_toView;

void main()
{
   const vec4  specColor = vec4 ( 1.0, 1.0, 1.0, 1.0 );
   const float specPower = 10.0;

//   vec4 diffColor = texture2D(texture0, vTexCoord) + vec4(mainColor, 1.);
//   vec4 diffColor = vec4 ( 0.0, 0.0, 1.0, 1.0 );
   vec4 diffColor = color;

   vec3 n2 = normalize ( v_normal );
   vec3 v2 = normalize ( v_toView );
   vec3 r  = reflect   ( -v2, n2 );

   vec3 l2 = normalize ( v_toLight );

   vec4 diff1 = diffColor * max ( dot ( n2, l2 ), 0.0 );
   vec4 spec1 = specColor * pow ( max ( dot ( l2, r ), 0.0 ), specPower );


   //   gl_FragColor = vec4(diff.rgb, 1.);
   //   gl_FragColor.rgb += spec.rgb;
   float opacitySpec = max(spec1.r, opacity);
   vec4 thePulse = vec4 ( .5, 0.0, 0.0, 1.0 ) * abs(sin(time));

   vec3 theColor = diff1.rgb + spec1.rgb + thePulse.rgb;

   gl_FragColor = vec4(theColor, opacitySpec);

}
