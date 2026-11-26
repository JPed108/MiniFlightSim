#version 120
varying vec4 litColor;
varying vec4 v_pos;

void main()
{
    gl_FragColor = litColor;
}