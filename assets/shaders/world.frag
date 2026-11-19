#version 330 core
in vec3 vPosition,vNormal;
in vec2 vUv;
in vec4 vLightPosition;
out vec4 fragmentColor;
uniform vec3 uColor,uEye,uSunDirection,uSkyColor;
uniform sampler2D uShadowMap;
uniform int uMaterial,uShadows,uWorldScene;
uniform float uEmission,uAlpha,uTime;
const float PI=3.14159265;
float hash(vec3 p){return fract(sin(dot(p,vec3(127.1,311.7,74.7)))*43758.5453);}
float noise(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3.0-2.0*f);
 return mix(mix(mix(hash(i),hash(i+vec3(1,0,0)),f.x),mix(hash(i+vec3(0,1,0)),hash(i+vec3(1,1,0)),f.x),f.y),
 mix(mix(hash(i+vec3(0,0,1)),hash(i+vec3(1,0,1)),f.x),mix(hash(i+vec3(0,1,1)),hash(i+vec3(1,1,1)),f.x),f.y),f.z);}
float shadow(vec3 N,vec3 L){
 if(uShadows==0)return 0.0;
 vec3 p=vLightPosition.xyz/vLightPosition.w*0.5+0.5;
 if(p.z>=1||p.z<=0||any(lessThan(p.xy,vec2(0)))||any(greaterThan(p.xy,vec2(1))))return 0.0;
 float bias=max(0.00018*(1.0-dot(N,L)),0.00006);
 vec2 texel=1.0/vec2(textureSize(uShadowMap,0));float sum=0;
 for(int x=-1;x<=1;++x)for(int y=-1;y<=1;++y)
  sum+=p.z-bias>texture(uShadowMap,p.xy+vec2(x,y)*texel*1.75).r?1.0:0.0;
 return sum/9.0;
}
vec3 display(vec3 x){x=(x*(2.51*x+0.03))/(x*(2.43*x+0.59)+0.14);return pow(clamp(x,0,1),vec3(1.0/2.2));}
void main(){
 vec3 N=normalize(vNormal),L=normalize(uSunDirection),V=normalize(uEye-vPosition);
 vec3 base=pow(max(uColor,vec3(0.01)),vec3(2.2));float rough=0.28,metal=0;
 float grain=noise(vPosition*7.0);
 if(uMaterial==1){ // Wood grain aligned along each plank.
  float g=sin(vPosition.x*3.0+noise(vPosition*0.8)*5.0+vPosition.z*35.0);
  base*=0.85+0.15*g+0.12*grain;rough=0.62;
 }else if(uMaterial==2){base*=0.83+0.25*grain;rough=0.85;
 }else if(uMaterial==3){base*=0.72+0.42*noise(vPosition*2.6);rough=0.95;
 }else if(uMaterial==4){base*=0.91+0.12*noise(vPosition*22.0);rough=0.9;
 }else if(uMaterial==5){
  vec2 p=vPosition.xz;
  float wave=sin(p.x*1.3+uTime*0.7)+sin(p.y*1.8-uTime*0.9)*0.5;
  N=normalize(N+vec3(cos(p.x*1.3+uTime*0.7)*0.10,0,cos(p.y*1.8-uTime*0.9)*0.07));
  base*=0.9+wave*0.08;rough=0.16;metal=0.15;
  float fres=pow(1-max(dot(N,V),0.0),4.0);
  base=mix(base,pow(uSkyColor,vec3(2.2))*0.7,fres*0.65);
  float shoreline=exp(-abs(p.y+14.0)*0.8);
  float foam=smoothstep(0.75,0.98,sin(p.y*4.0+uTime*1.6+sin(p.x*0.7)))*shoreline;
  base=mix(base,vec3(0.65),foam*0.7);
 }else if(uMaterial==6){base*=0.96+0.05*grain;rough=0.72;
 }else if(uMaterial==7){base*=0.74+0.38*noise(vPosition*6.0);rough=0.9;
 }else if(uMaterial==8){metal=0.75;rough=0.35;}
 vec3 H=normalize(V+L);float NoL=max(dot(N,L),0),NoV=max(dot(N,V),0.001),NoH=max(dot(N,H),0);
 float a=rough*rough,a2=a*a;
 float d=NoH*NoH*(a2-1.0)+1.0;
 float D=a2/max(PI*d*d,0.0001);
 float k=(rough+1.0)*(rough+1.0)/8.0;
 float G=(NoV/(NoV*(1-k)+k))*(NoL/(NoL*(1-k)+k));
 vec3 F0=mix(vec3(0.04),base,metal);
 vec3 F=F0+(1-F0)*pow(1.0-max(dot(H,V),0),5.0);
 vec3 spec=D*G*F/max(4.0*NoV*NoL,0.001);
 vec3 diffuse=(1-F)*(1-metal)*base/PI;
 float occ=shadow(N,L);
 vec3 lightColor=mix(vec3(1.0,0.72,0.45),vec3(1.0,0.91,0.77),smoothstep(0.12,0.7,L.y));
 vec3 hemi=mix(vec3(0.20,0.17,0.12),vec3(0.50,0.65,0.83),N.y*0.5+0.5);
 vec3 lit=base*hemi*0.65+(diffuse+spec)*lightColor*3.4*NoL*(1.0-occ*0.88);
 if(uMaterial==7)lit+=base*0.13*max(dot(-N,L),0.0); // gentle leaf translucency
 lit+=base*uEmission*3.0;
 if(uWorldScene!=0){
  float distance=length(uEye-vPosition);
  float fog=1.0-exp(-distance*distance*0.00011);
  lit=mix(lit,vec3(0.49,0.61,0.70),clamp(fog,0.0,0.82));
 }
 fragmentColor=vec4(display(lit),uAlpha);
}
