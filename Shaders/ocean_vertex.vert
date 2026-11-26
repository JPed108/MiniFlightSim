#version 120
varying vec4 litColor;
varying vec4 v_pos;

void main()
{
    v_pos = gl_Vertex;
    vec4 eyePosition = gl_ModelViewMatrix * gl_Vertex;
    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
    vec3 lightVector = gl_LightSource[0].position.xyz
                        - eyePosition.xyz * gl_LightSource[0].position.w;
    float diffuse = max(dot(normal, normalize(lightVector)), 0.0);
    vec3 ambient = gl_LightModel.ambient.rgb
                    + gl_LightSource[0].ambient.rgb;
    vec3 rgb = gl_Color.rgb
                * (ambient + gl_LightSource[0].diffuse.rgb * diffuse)
                + gl_FrontMaterial.emission.rgb;

    litColor = vec4(rgb, gl_Color.a);
    gl_Position = gl_ProjectionMatrix * eyePosition;
}