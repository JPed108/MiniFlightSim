#version 120

uniform float uHeightMapScale;
uniform float uPlanetRadius;

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

    float landFactor = smoothstep(0.22, 0.23, vHeight);
    float heightAboveSea = max(vHeight - 0.22, 0.0) * landFactor;
    vec3 vertexPos = normalize(vObjPos) * (1.0 + heightAboveSea * uHeightMapScale);


    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
    vec3 sphereNormal = normalize(gl_NormalMatrix * vObjPos);
    vNormal = mix(sphereNormal, normal, smoothstep(0.22, 0.23, vHeight));
    vViewPos = (gl_ModelViewMatrix * vec4(vertexPos, 1.0)).xyz;
    gl_Position = gl_ModelViewProjectionMatrix  * vec4(vertexPos, 1.0);
}