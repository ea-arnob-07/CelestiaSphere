#version 330 core
in vec3 vWorldPosition;
in vec3 vNormal;
in vec3 vLocalPosition;
in vec2 vTexCoord;

out vec4 FragColor;

uniform vec3 uCameraPosition;
uniform vec3 uSunPosition;
uniform vec3 uBaseColor;
uniform vec3 uAccentColor;
uniform int uBodyKind;
uniform bool uIsSun;
uniform bool uSelected;
uniform float uTime;

float hash31(vec3 p) {
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

float valueNoise3D(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float n000 = hash31(i + vec3(0,0,0));
    float n100 = hash31(i + vec3(1,0,0));
    float n010 = hash31(i + vec3(0,1,0));
    float n110 = hash31(i + vec3(1,1,0));
    float n001 = hash31(i + vec3(0,0,1));
    float n101 = hash31(i + vec3(1,0,1));
    float n011 = hash31(i + vec3(0,1,1));
    float n111 = hash31(i + vec3(1,1,1));
    float nx00 = mix(n000, n100, f.x);
    float nx10 = mix(n010, n110, f.x);
    float nx01 = mix(n001, n101, f.x);
    float nx11 = mix(n011, n111, f.x);
    return mix(mix(nx00, nx10, f.y), mix(nx01, nx11, f.y), f.z);
}

float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 6; ++i) {
        value += valueNoise3D(p) * amplitude;
        p = p * 2.03 + vec3(13.1, 7.7, 5.3);
        amplitude *= 0.5;
    }
    return value;
}

vec3 proceduralSurface(out float oceanMask, out float cloudMask, out float cityMask) {
    vec3 p = normalize(vLocalPosition);
    float latitude = asin(clamp(p.y, -1.0, 1.0));
    float longitude = atan(p.z, p.x);
    float fineNoise = fbm(p * 8.0);
    oceanMask = 0.0;
    cloudMask = 0.0;
    cityMask = 0.0;

    if (uBodyKind == 0) {
        float cells = fbm(p * 5.0 + vec3(uTime * 0.03, 0.0, 0.0));
        float bands = 0.5 + 0.5 * sin(latitude * 18.0 + cells * 7.0 + uTime * 0.25);
        float flares = smoothstep(0.55, 0.98, fbm(p * 12.0 + vec3(uTime * 0.05)));
        return mix(uAccentColor * 1.15, uBaseColor * 1.45, smoothstep(0.15, 0.9, bands)) + flares * vec3(0.25, 0.15, 0.05);
    }
    if (uBodyKind == 1 || uBodyKind == 5 || uBodyKind == 6) {
        float crater = smoothstep(0.34, 0.80, fineNoise);
        float ridges = abs(sin(longitude * 8.0 + fbm(p * 3.0) * 5.0));
        float dust = fbm(p * 18.0);
        return mix(uBaseColor * (0.55 + fineNoise * 0.72), uAccentColor, crater * 0.62 + ridges * 0.10) + dust * 0.05;
    }
    if (uBodyKind == 2) {
        float continents = fbm(p * 2.55 + vec3(2.0, 5.0, 1.0));
        continents += 0.18 * fbm(p * 7.5);
        float landMask = smoothstep(0.50, 0.60, continents);
        oceanMask = 1.0 - landMask;
        float elevation = smoothstep(0.58, 0.90, fbm(p * 11.0));
        vec3 deepOcean = vec3(0.02, 0.16, 0.40);
        vec3 shallowOcean = vec3(0.08, 0.38, 0.74);
        vec3 ocean = mix(deepOcean, shallowOcean, smoothstep(0.20, 0.82, fineNoise + p.y * 0.08));
        vec3 equatorialGreen = vec3(0.16, 0.56, 0.23);
        vec3 temperateGreen = vec3(0.28, 0.50, 0.22);
        vec3 aridBrown = vec3(0.56, 0.44, 0.22);
        float climate = smoothstep(0.10, 0.80, abs(p.y));
        vec3 lowLand = mix(equatorialGreen, temperateGreen, climate);
        vec3 highLand = mix(lowLand, aridBrown, elevation * 0.75);
        vec3 polarCap = vec3(0.92, 0.96, 1.0);
        vec3 land = mix(lowLand, highLand, elevation);
        float ice = smoothstep(0.70, 0.95, abs(p.y) + elevation * 0.12);
        cloudMask = smoothstep(0.63, 0.80, fbm(p * 14.0 + vec3(0.0, uTime * 0.008, uTime * 0.004)));
        cloudMask *= 0.72 + 0.28 * smoothstep(0.1, 0.85, fbm(p * 22.0));
        cityMask = landMask * smoothstep(0.18, 0.45, fineNoise) * (1.0 - smoothstep(0.55, 0.85, abs(p.y)));
        vec3 surface = mix(ocean, land, landMask);
        surface = mix(surface, polarCap, ice);
        surface = mix(surface, vec3(0.97, 0.98, 1.0), cloudMask * 0.55);
        return surface;
    }
    if (uBodyKind == 3) {
        float bands = 0.5 + 0.5 * sin(latitude * 36.0 + fbm(vec3(longitude * 0.8, latitude * 4.0, 0.0)) * 5.0);
        float turbulence = fbm(vec3(longitude * 2.0, latitude * 12.0, uTime * 0.015));
        vec3 gas = mix(uBaseColor, uAccentColor, bands * 0.62 + turbulence * 0.20);
        float storm = exp(-70.0 * pow(latitude + 0.28, 2.0)) * exp(-8.0 * pow(sin(longitude * 0.5 - 1.0), 2.0));
        return mix(gas, vec3(0.72, 0.30, 0.12), storm * 0.75);
    }
    if (uBodyKind == 4) {
        float bands = 0.5 + 0.5 * sin(latitude * 18.0 + fineNoise * 2.0);
        float haze = smoothstep(0.25, 0.82, fbm(p * 10.0 + vec3(0.0, uTime * 0.01, 0.0)));
        return mix(uBaseColor, uAccentColor, bands * 0.35 + fineNoise * 0.15) + haze * vec3(0.03, 0.07, 0.10);
    }
    if (uBodyKind == 7) {
        float rim = pow(1.0 - max(dot(normalize(vNormal), normalize(uCameraPosition - vWorldPosition)), 0.0), 3.4);
        float swirl = fbm(p * 24.0 + vec3(uTime * 0.02, -uTime * 0.015, 0.0));
        vec3 ringGlow = mix(vec3(0.30, 0.50, 1.0), vec3(1.0, 0.55, 0.15), swirl);
        return vec3(0.002, 0.002, 0.008) + ringGlow * rim * 0.85;
    }
    if (uBodyKind == 8) {
        float panel = step(0.72, abs(sin(vTexCoord.x * 42.0))) * 0.18;
        return mix(uBaseColor, uAccentColor, panel + smoothstep(0.65, 0.95, vTexCoord.y) * 0.35);
    }
    return uBaseColor;
}

void main() {
    vec3 normal = normalize(vNormal);
    vec3 viewDirection = normalize(uCameraPosition - vWorldPosition);
    float oceanMask;
    float cloudMask;
    float cityMask;
    vec3 surface = proceduralSurface(oceanMask, cloudMask, cityMask);

    if (uIsSun) {
        float rim = pow(1.0 - max(dot(normal, viewDirection), 0.0), 2.5);
        vec3 emissive = surface * (1.35 + rim * 1.6);
        FragColor = vec4(emissive, 1.0);
        return;
    }

    vec3 lightDirection = normalize(uSunPosition - vWorldPosition);
    float diffuse = max(dot(normal, lightDirection), 0.0);
    vec3 halfDirection = normalize(lightDirection + viewDirection);
    float specularPower = uBodyKind == 2 ? 96.0 : (uBodyKind == 3 || uBodyKind == 4 ? 30.0 : 24.0);
    float specular = pow(max(dot(normal, halfDirection), 0.0), specularPower);
    float nightRim = pow(1.0 - max(dot(normal, viewDirection), 0.0), 3.0);
    float sunDistance = max(length(uSunPosition - vWorldPosition), 1.0);
    float lightFalloff = clamp(24.0 / sqrt(sunDistance), 0.32, 1.0);

    vec3 color = surface * (0.055 + diffuse * lightFalloff * 1.20);
    float oceanSpec = oceanMask * smoothstep(0.0, 0.8, diffuse);
    color += vec3(1.0, 0.88, 0.70) * specular * (0.18 + oceanSpec * 0.55) * lightFalloff;
    color += uAccentColor * nightRim * 0.045;

    if (uBodyKind == 2) {
        float nightSide = smoothstep(0.05, 0.85, -dot(normal, lightDirection));
        vec3 cityLights = vec3(1.0, 0.72, 0.32) * cityMask * nightSide * 0.48;
        color += cityLights;
        color += vec3(0.94, 0.97, 1.0) * cloudMask * 0.05;
    }

    if (uBodyKind == 7) {
        float lens = pow(1.0 - max(dot(normal, viewDirection), 0.0), 4.0);
        color = mix(vec3(0.001, 0.001, 0.004), color, 0.92);
        color += vec3(0.15, 0.34, 0.90) * lens * 0.55;
        color += vec3(1.00, 0.58, 0.18) * lens * 0.28;
    }
    FragColor = vec4(color, 1.0);
}
