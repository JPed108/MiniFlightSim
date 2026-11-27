#version 120
varying vec3 vViewPos;
varying vec3 vNormal;


void main()
{
    vViewPos = (gl_ModelViewMatrix * gl_Vertex).xyz;

    gl_Position = ftransform();
}