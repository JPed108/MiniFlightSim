#version 120
varying vec3 vViewPos;

uniform vec3 uCameraPos;
uniform vec3 uPlanetCenter;
uniform vec3 uSunPos;
uniform float uPlanetRadius;


void main()
{
    vec3 localUp = normalize(uCameraPos - uPlanetCenter);
    vec3 sunDir = normalize(uSunPos - uCameraPos);
    vec3 viewDir = normalize(vViewPos - uCameraPos);

    vec3 daylightColor = vec3(0.510, 0.784, 0.898);
    vec3 sunsetColor   = vec3(0.992, 0.369, 0.325);
    vec3 sunColor      = vec3(1.0, 1.0, 1.0);
    vec3 nightSky      = vec3(0.016, 0.102, 0.251);
    
    float ratio = (uPlanetRadius / (length(uCameraPos - uPlanetCenter)));

    float sunsetAngle = -sqrt(max((1.0 - ratio * ratio), 0.0));

    float sunHeight = dot(localUp, sunDir) - sunsetAngle;

    float dayFactor = smoothstep(-0.18, 0.05, sunHeight);
    vec3 skyColor = mix(nightSky, daylightColor, dayFactor);
    float sunsetFactor = smoothstep(-0.16, 0.0, sunHeight) * (1.0 - smoothstep(0.0, 0.14, sunHeight));

    float alignment = clamp(dot(viewDir, sunDir), 0.0, 1.0);
    float sunsetGlow = pow(alignment, 32.0) * 0.85 * sunsetFactor;
    skyColor = mix(skyColor, sunsetColor, sunsetGlow);   

    float viewOverHorizon = dot(localUp, viewDir) - sunsetAngle;
    float horizontalBand = exp(-max(viewOverHorizon, 0.0) * 16.0);

    float sunFacing      = dot(viewDir, sunDir) * 0.5 + 0.5;
    float azimuthWeight  = mix(0.20, 1.0, pow(sunFacing, 2.0));

    float bandStr = clamp(horizontalBand * sunsetFactor * azimuthWeight, 0.0, 1.0);

    skyColor = mix(skyColor, sunsetColor, bandStr);  


    float sunVisibleFactor = smoothstep(-0.1, 0.0, sunHeight);

    float sunDisk = smoothstep(0.999, 0.9996, max(alignment, 0.0));
    float glow1 = pow(alignment, 256.0) * 0.75;
    float glow2 = pow(alignment, 64.0) * 0.75;
    float halo = (glow1 + glow2) * sunVisibleFactor;

    vec3 haloColor = mix(sunColor, sunsetColor, sunsetFactor );

    vec3 color = mix(skyColor, haloColor, clamp(halo, 0.0, 1.0));
    color = mix(color, sunColor, sunDisk);

    gl_FragColor = vec4(color, 1.0);


}