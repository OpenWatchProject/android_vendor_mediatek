
precision mediump float;

/* Transformation uniforms */
uniform highp mat4 u_t_modelViewProjection; // Model to projection space transform
uniform vec4 u_uvCoordOffsetScale;

attribute vec4 a_position;    // Position in model space

// Texture coordinate for sphere-mapped reflections
varying mediump vec2 v_texCoord0;

void main()
{
  gl_Position = u_t_modelViewProjection * a_position;
  v_texCoord0.x = a_position.x + 0.5;
  v_texCoord0.y = 1.0 - (a_position.y + 0.5);
}
