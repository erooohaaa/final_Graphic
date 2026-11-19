#version 330 core
in vec2 vNdc;
out vec4 fragmentColor;
uniform mat4 uInverseViewProjection;
uniform vec3 uEye,uSunDirection,uSkyColor;
uniform float uTime;
float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float noise(vec2 p){vec2 i=floor(p),f=fract(p);f=f*f*(3.0-2.0*f);
 return mix(mix(hash(i),hash(i+vec2(1,0)),f.x),mix(hash(i+vec2(0,1)),hash(i+vec2(1)),f.x),f.y);}
vec3 display(vec3 x){x=(x*(2.51*x+0.03))/(x*(2.43*x+0.59)+0.14);return pow(clamp(x,0,1),vec3(1.0/2.2));}
void main(){
 vec4 w=uInverseViewProjection*vec4(vNdc,1,1);vec3 ray=normalize(w.xyz/w.w-uEye);
 float y=max(ray.y,0.0);
 vec3 sky=mix(vec3(0.63,0.72,0.78),uSkyColor,smoothstep(0.0,0.65,y));
 float toward=clamp(dot(ray,uSunDirection),0.0,1.0);
 sky+=vec3(0.5,0.23,0.06)*pow(toward,12.0)*0.32;
 if(ray.y>0.015){
  vec2 p=ray.xz/max(ray.y+0.12,0.12)*1.1+vec2(uTime*0.004,0);
  float cloud=noise(p)*0.55+noise(p*2.02)*0.28+noise(p*4.01)*0.17;
  float c=smoothstep(0.56,0.76,cloud)*smoothstep(0.015,0.15,ray.y);
  sky=mix(sky,vec3(0.94,0.91,0.83),c*0.75);
 }
 // Angular disk at infinity: camera translation can never bring it into the scene.
 float disk=smoothstep(cos(0.009),cos(0.006),toward);
 sky+=vec3(8.0,5.5,2.7)*disk+vec3(0.65,0.38,0.12)*pow(toward,650.0);
 fragmentColor=vec4(display(sky),1);
}
