// Very basic shaded rotating cube

#version 460

layout (location=0) uniform vec4 params;
out vec4 outCol;

mat2 rot(float a) {
    float c = cos(a);
    float s = sin(a);
    return mat2(c, -s, s, c);
}

float sdBox(vec3 p, vec3 b){
    vec3 q = abs(p) - b;
    return length(max(q, 0.)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float map(vec3 p){
    float a = 1.5*params.z;
    p.xz *= rot(a);
    p.yx *= rot(a);
    p.zy *= rot(a);
    float db = sdBox(p, vec3(0.5)) - 0.03;
    return db;
}

float raymarch(vec3 ro, vec3 rd) {
    float t = 0.;
    for(float i = 0.; i < 32.; ++i){
        float d = map(ro + rd*t);
        if(d < 0.001){
            return t;
        }
        t += d;
    }
    return -1.;
}

vec3 normal(vec3 p){
    vec2 h = vec2(0.001, 0.);
    return normalize(vec3(
        map(p+h.xyy) - map(p-h.xyy),
        map(p+h.yxy) - map(p-h.yxy),
        map(p+h.yyx) - map(p-h.yyx)
    ));
}

void main()
{
    vec2 uv = gl_FragCoord.xy/params.xy;
    uv -= 0.5;
    uv.x *= params.x/params.y;
    vec3 ro = vec3(0.,0.,-5.);
    vec3 rd = normalize(vec3(uv, 1.));
    vec3 col = vec3(0.);
    float t = raymarch(ro, rd);
    if(t > 0.){
        vec3 ldir = normalize(vec3(1.,1.,-1.));
        vec3 p = ro + rd*t;
        vec3 n = normal(p);
        vec3 h = normalize(ldir-rd);
        float fd = max(0., dot(n,ldir));
        float fs = pow(max(0.,dot(n,h)),16.0);
        float fr = pow(1.-max(0.,dot(n,-rd)), 5.);
        col = vec3(0.65,0.6,0.5)*fd + vec3(0.9,0.8,0.7)*fs*(0.5+0.5*fr);
        col += vec3(0.1,0.2,0.3)*0.2*(max(0., -dot(n,ldir))+fr);
    } else {
        col = vec3(0.5,0.6,0.7);
    }
    col = pow(col, vec3(1./2.2));
    outCol = vec4(col,1.0);
}