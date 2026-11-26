#version 120

varying float vHeight;
varying vec3 vNormal;
varying vec3 vObjPos;
varying vec3 vViewPos;
varying vec3 vObjNormal;

uniform float uTime;


// Função de ruído obtida de: https://gist.github.com/clauswilke/c39feab04463b779200dfa68d6890b5a

//	Simplex 3D Noise 
//	by Ian McEwan, Ashima Arts
//

#define NUM_OCTAVES 4

vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //  x0 = x0 - 0. + 0.0 * C 
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
  i = mod(i, 289.0 ); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}

float fbm(vec3 x) {
	float v = 0.0;
	float a = 0.5;
	vec3 shift = vec3(100);
	for (int i = 0; i < NUM_OCTAVES; ++i) {
		v += a * snoise(x);
		x = x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}

void main()
{
    float h = clamp(vHeight, 0.0, 1.0);

    vec3 dir = normalize(vObjPos);


    float noise = fbm(dir * 8.0);
    // Adicionamos ruído na altura para quebrar a transição entre cores
    float hNoise = h + noise * 0.005;
    // E adicionamos ruído na latitude para quebrar a transição para o gelo
    float lat = degrees(asin(clamp(abs(dir.y), 0.0, 1.0))) + noise * 5.0;

    vec3 deepWater    = vec3(0.04, 0.14, 0.36);
    vec3 shallowWater = vec3(0.10, 0.35, 0.60);
    vec3 sand         = vec3(0.76, 0.70, 0.50);
    vec3 grass        = vec3(0.18, 0.45, 0.16);
    vec3 grass2       = vec3(0.043, 0.302, 0.016);
    vec3 rock         = vec3(0.42, 0.39, 0.36);
    vec3 snow         = vec3(0.95, 0.96, 0.98);
    vec3 seaIce       = vec3(0.352, 0.731, 0.950);
    // Misturamos os dois tipos de grama com base no ruído para quebrar a cor uniforme
    vec3 grassColor = mix(grass, grass2, smoothstep(0.3, 0.7, (noise + 1.0) * 0.5));

    vec3 albedo = mix(deepWater, shallowWater, smoothstep(0.1, 0.2, hNoise));
    albedo = mix(albedo, sand,  smoothstep(0.22, 0.23, hNoise));
    albedo = mix(albedo, grassColor, smoothstep(0.235, 0.24, hNoise));
    albedo = mix(albedo, snow,  smoothstep(0.55, 0.7, hNoise));

    // Gelo devido a latitude
    // Está na latitude > 70,  < 76 e na água?
    float seaIceFactor = smoothstep(65.0, 70.0, lat) * (1.0 - smoothstep(0.22, 0.23, hNoise));

    // Se seaIceFactor = 0, não temos gelo, = 1 gelo, entre 0 e 1 transição
    albedo = mix(albedo, seaIce, seaIceFactor);

    // Gelo devido à latitude.
    // 55 < lat < 60 & é solo
    float snowFactor = smoothstep(55.0, 65.0, lat) * smoothstep(0.22, 0.23, h);
    albedo = mix(albedo, snow, snowFactor);

    // Vetor normal à superfície no espaço do objeto
    vec3 N = normalize(vObjNormal);
    // Vetor dir é a normal em relação à superfície do planeta. O ângulo é dado por:
    float inc = degrees(acos(clamp(dot(dir, N), -1.0, 1.0)));
    // Se o ângulo for muito elevado, pintamos o terreno com cor de rocha
    float rockCenter = 45.0 + noise * 5.0;  
    float rockFactor = smoothstep(rockCenter - 10.0, rockCenter + 10.0, inc) * (1.0 - smoothstep(65.0, 70.0, lat));

    albedo = mix(albedo, rock, rockFactor);

    // Ondas na praia
    // Criamos uma função seno variando no tempo, regiões mais distantes dos continentes possuem alturas menores, ondas chegam primeiro
    // Máscara de regiões próximas às costas
    // Frequência das ondas aumentam mais próximas da costa
    // Fase das ondas depende da produndidade
    float noise2 = snoise(dir * 60.0 + vec3(uTime * 0.15));
    
    float coast = (1.0 - smoothstep(0.21, 0.23, h + noise * 0.004)) * smoothstep(0.18, 0.19, h + noise * 0.004);
    float phase = h * 180.0 - uTime * 3.0 + noise * 5.0 + lat * 0.2;

    float wave = pow(max(sin(phase) - (0.3 + noise2 * 0.3), 0.0), 4.0);

    vec3 wave_color = mix(shallowWater, vec3(1.0), wave);
    albedo = mix(albedo, wave_color, coast);

    // Vetor em direção à câmera
    vec3 V = normalize(-vViewPos);
    // Vetor em direção à fonte de luz
    vec3 L = normalize(gl_LightSource[0].position.xyz - vViewPos);


    //Lambertiano puro
    float diff = max(dot(vNormal , L), 0.0);

    // Blinn phong
    vec3 H = normalize(V + L);

    float NdotH = max(dot(vNormal , H), 0.0) * (1.0 - smoothstep(0.22, 0.23, hNoise));

    float spec = (diff > 0.0) ? pow(NdotH, 64.0) : 0.0;

    vec3 finalColor = albedo * (0.2 + 0.8 * diff) + 0.4*spec;
    gl_FragColor = vec4(finalColor, 1.0);

}