#version 120

uniform float uHeightMapScale;

varying float vHeight;
varying vec3 vNormal;
varying vec3 vObjNormal;

varying vec3 vObjPos;
varying vec3 vViewPos;


void main()
{
    vObjPos = gl_Vertex.xyz;
    vObjNormal = gl_Normal;
    vHeight = (length(gl_Vertex.xyz) - 1.0) / uHeightMapScale;
    vNormal = normalize(gl_NormalMatrix * gl_Normal);
    vViewPos = (gl_ModelViewMatrix * gl_Vertex).xyz;
    gl_Position = ftransform();
}