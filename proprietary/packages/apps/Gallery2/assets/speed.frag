
precision mediump float;

uniform sampler2D u_m_diffuseTexture;
uniform sampler2D u_m_newTexture;
uniform vec4 u_m_diffuseColour;

uniform float u_level;
uniform float u_step;
uniform float u_fadeoutstep;
uniform float u_center_x;
uniform float u_center_y;
uniform float u_m_opacity;
varying mediump vec2 v_texCoord0;

void main()
{
  vec2 center;
  float factor = 0.3 * u_step;

  vec2 new_coord;
  float temp_factor;
  float base = (factor / u_level);
  center.x = u_center_x;
  center.y = u_center_y;
  gl_FragColor = texture2D(u_m_diffuseTexture, v_texCoord0);
  for (float i=1.0 ; i<=u_level ; i++) {
    temp_factor = i * base;
    new_coord = mix(v_texCoord0, center, temp_factor); 
    gl_FragColor += texture2D(u_m_diffuseTexture, new_coord);
  }
  gl_FragColor = mix(texture2D(u_m_newTexture, v_texCoord0), gl_FragColor / (u_level + 1.0), u_fadeoutstep);
  //gl_FragColor = u_m_diffuseColour * gl_FragColor;
  gl_FragColor.a *= u_m_opacity;
}
