#include "Environment.hpp"
#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>

void SunPosition::move(float horizontal,float vertical,float dt) {
    float safeDt=std::clamp(dt,0.0f,0.1f);
    azimuth_ = std::remainder(azimuth_+horizontal*safeDt*0.7f,glm::two_pi<float>());
    elevation_=std::clamp(elevation_+vertical*safeDt*0.5f,glm::radians(8.0f),glm::radians(82.0f));
}
void SunPosition::reset() { azimuth_=-0.80f; elevation_=0.70f; }
glm::vec3 SunPosition::direction() const {
    return {std::sin(azimuth_)*std::cos(elevation_),std::sin(elevation_),
        -std::cos(azimuth_)*std::cos(elevation_)};
}
float SunPosition::azimuthDegrees() const {return glm::degrees(azimuth_);}
float SunPosition::elevationDegrees() const {return glm::degrees(elevation_);}
const char* Environment::name(Location l) {
    switch(l) {case Location::Park:return "GARDEN PARK";case Location::Beach:return "SUNLIT COAST";
        case Location::Winter:return "WINTER GARDEN";}
    return "UNKNOWN";
}
glm::vec3 Environment::skyColor(Location l) {
    return l==Location::Winter?glm::vec3(0.49f,0.65f,0.83f):glm::vec3(0.29f,0.55f,0.81f);
}
namespace {
struct Builder {
    std::vector<SceneObject> items;
    void add(MeshShape shape,glm::vec3 p,glm::vec3 size,glm::vec3 c,Surface m,
             glm::vec3 rot=glm::vec3(0),float emission=0,bool cast=true) {
        SceneObject b;b.shape=shape;b.center=p;b.size=size;b.color=c;b.material=m;
        b.rotation=rot;b.emission=emission;b.castsShadow=cast;items.push_back(b);
    }
    void box(glm::vec3 p,glm::vec3 s,glm::vec3 c,Surface m,glm::vec3 rot=glm::vec3(0)) {
        add(MeshShape::Box,p,s,c,m,rot);
    }
    void ball(glm::vec3 p,glm::vec3 s,glm::vec3 c,Surface m) {add(MeshShape::Sphere,p,s,c,m);}
    void pole(glm::vec3 p,float h,float radius,glm::vec3 c,Surface m) {
        add(MeshShape::Cylinder,p,{radius*2,h,radius*2},c,m);
    }
    void tree(float x,float z,float scale,bool winter,int seed) {
        float base=-1.48f;
        pole({x,base+1.7f*scale,z},3.4f*scale,0.16f*scale,{0.23f,0.12f,0.055f},Surface::Wood);
        // Smooth, layered canopies rather than cube trees.
        for(int j=0;j<5;++j) {
            float a=j*2.39996f+seed;
            glm::vec3 p(x+std::cos(a)*0.75f*scale,base+(3.6f+0.28f*(j%3))*scale,z+std::sin(a)*0.7f*scale);
            glm::vec3 leaf(0.10f+0.025f*(j%3),0.27f+0.028f*(seed%4),0.055f);
            ball(p,{2.6f*scale,2.35f*scale,2.6f*scale},leaf,Surface::Leaves);
            if(winter)ball(p+glm::vec3(0,0.75f*scale,0),{2.35f*scale,0.65f*scale,2.35f*scale},
                {0.86f,0.92f,0.97f},Surface::Snow);
        }
    }
    void bench(glm::vec3 p,float yaw) {
        auto part=[&](glm::vec3 offset,glm::vec3 s,glm::vec3 c,Surface m) {
            float a=glm::radians(yaw);
            glm::vec3 q(std::cos(a)*offset.x+std::sin(a)*offset.z,offset.y,
                -std::sin(a)*offset.x+std::cos(a)*offset.z);
            box(p+q,s,c,m,{0,yaw,0});
        };
        for(int i=0;i<4;++i)part({0,-0.65f,-0.31f+i*0.21f},{2.5f,0.13f,0.17f},{0.48f,0.23f,0.08f},Surface::Wood);
        for(int i=0;i<3;++i)part({0,-0.25f+i*0.2f,0.48f},{2.5f,0.14f,0.12f},{0.48f,0.23f,0.08f},Surface::Wood);
        for(float side:{-0.9f,0.9f}) {
            part({side,-1.06f,0},{0.12f,0.8f,0.75f},{0.085f,0.10f,0.10f},Surface::Metal);
            part({side,-0.28f,0.42f},{0.11f,1.1f,0.11f},{0.085f,0.10f,0.10f},Surface::Metal);
        }
    }
};
}
std::vector<SceneObject> Environment::build(Location l,bool high) {
    Builder b;bool winter=l==Location::Winter;
    glm::vec3 green(0.19f,0.34f,0.09f),sand(0.72f,0.55f,0.32f),snow(0.83f,0.9f,0.96f);
    b.box({0,-1.8f,0},{180,0.55f,180},l==Location::Beach?sand:winter?snow:green,
        l==Location::Beach?Surface::Sand:winter?Surface::Snow:Surface::Grass);
    // A solid wood table beneath the original playing surface.
    b.box({0,-0.60f,0},{9.2f,0.3f,9.2f},{0.31f,0.15f,0.065f},Surface::Wood);
    for(float x:{-3.5f,3.5f})for(float z:{-3.5f,3.5f})
        b.box({x,-1.1f,z},{0.45f,1.0f,0.45f},{0.22f,0.11f,0.045f},Surface::Wood);
    // The playing table sits on a paved patio, not suspended in empty space.
    for(int x=-5;x<=5;++x)for(int z=-5;z<=5;++z) {
        if(std::abs(x)<4&&std::abs(z)<4)continue;
        float variation=0.02f*((x*x+z*z)%3);
        b.box({x*1.04f,-1.43f,z*1.04f},{1.0f,0.14f,1.0f},
            winter?glm::vec3(0.65f+variation,0.73f+variation,0.78f+variation):
            glm::vec3(0.40f+variation,0.38f+variation,0.32f+variation),Surface::Stone);
    }
    if(l==Location::Beach) {
        b.box({0,-1.47f,-49},{180,0.10f,70},{0.025f,0.29f,0.40f},Surface::Water);
        b.box({0,-1.48f,-12},{180,0.05f,4.0f},{0.53f,0.49f,0.34f},Surface::Sand);
        for(float x:{-10.0f,10.0f})for(float z:{-5.0f,5.0f}) {
            b.pole({x,0.0f,z},3.0f,0.055f,{0.47f,0.30f,0.12f},Surface::Wood);
            b.add(MeshShape::Cone,{x,1.66f,z},{4.4f,0.85f,4.4f},x<0?glm::vec3(0.80f,0.28f,0.13f):glm::vec3(0.16f,0.48f,0.51f),Surface::Plastic);
            b.pole({x,2.16f,z},0.2f,0.06f,{0.82f,0.72f,0.5f},Surface::Wood);
            b.box({x+0.8f,-1.03f,z+1.5f},{1.0f,0.22f,2.3f},{0.78f,0.72f,0.54f},Surface::Wood);
            b.box({x+0.8f,-0.76f,z+2.25f},{1.0f,0.6f,0.15f},{0.85f,0.79f,0.6f},Surface::Wood,{-28,0,0});
        }
        for(int i=0;i<10;++i) {
            float x=i%2?-15.0f:15.0f,z=-12.0f+(i/2)*7;
            b.pole({x,0.5f,z},4.0f,0.15f,{0.36f,0.23f,0.10f},Surface::Wood);
            for(int j=0;j<7;++j) {
                float a=j*glm::two_pi<float>()/7;
                b.add(MeshShape::Sphere,{x+std::sin(a)*1.1f,2.6f,z+std::cos(a)*1.1f},
                    {0.65f,0.22f,3.5f},{0.12f,0.34f,0.12f},Surface::Leaves,{12,glm::degrees(a),0});
            }
        }
        for(int i=0;i<(high?28:12);++i) {
            float x=float((i*37)%300)/10-15,z=float((i*53)%190)/10-8;
            if(std::abs(x)<6&&std::abs(z)<6)continue;
            b.ball({x,-1.37f,z},{0.2f,0.12f,0.3f},{0.58f,0.53f,0.43f},Surface::Stone);
        }
    } else {
        // A cross-shaped park walk and decorative low edging.
        for(int i=-21;i<=21;++i)for(int j=-1;j<=1;++j) {
            if(std::abs(i)<6)continue;
            glm::vec3 col=winter?glm::vec3(0.60f,0.68f,0.74f):glm::vec3(0.45f,0.40f,0.30f);
            b.box({i*0.86f,-1.44f,j*0.72f},{0.82f,0.1f,0.68f},col,Surface::Stone);
            b.box({j*0.72f,-1.44f,i*0.86f},{0.68f,0.1f,0.82f},col,Surface::Stone);
        }
        for(float x:{-8.8f,8.8f}) {b.bench({x,0,3.0f},x<0?90.0f:-90.0f);b.bench({x,0,-4.5f},x<0?90.0f:-90.0f);}
        for(int i=0;i<(high?18:10);++i) {
            float a=i*2.39996f,rad=13.5f+(i%3)*5.0f;
            b.tree(std::cos(a)*rad,std::sin(a)*rad,0.85f+(i%4)*0.13f,winter,i);
        }
        // Shrub beds keep the foreground open and add layered silhouettes.
        for(int i=0;i<20;++i) {
            float a=i*glm::two_pi<float>()/20;
            float x=std::cos(a)*9.7f,z=std::sin(a)*9.7f;
            if(std::abs(x)<3||std::abs(z)<3)continue;
            b.ball({x,-1.04f,z},{1.8f,0.9f,1.5f},winter?snow:glm::vec3(0.12f,0.28f,0.085f),winter?Surface::Snow:Surface::Leaves);
            if(!winter)for(int k=0;k<3;++k)
                b.ball({x+(k-1)*0.3f,-0.50f,z},{0.18f,0.18f,0.18f},
                    k%2?glm::vec3(0.85f,0.35f,0.28f):glm::vec3(0.85f,0.66f,0.28f),Surface::Leaves);
        }
        if(high&&!winter)for(int i=0;i<230;++i) {
            float x=float((i*73)%460)/10-23,z=float((i*97)%460)/10-23;
            if((std::abs(x)<6&&std::abs(z)<6)||std::abs(x)<1.5f||std::abs(z)<1.5f)continue;
            b.add(MeshShape::Cone,{x,-1.29f,z},{0.13f,0.42f,0.13f},
                {0.19f+0.025f*(i%3),0.39f,0.095f},Surface::Grass,{0,float(i*17),float(i%13)-6});
        }
    }
    if(winter) {
        // Layered conical evergreen with baubles and warm garland lights.
        b.pole({0,-0.4f,-10.5f},2.2f,0.25f,{0.3f,0.16f,0.06f},Surface::Wood);
        for(int layer=0;layer<5;++layer) {
            float w=4.8f-layer*0.78f,y=-0.1f+layer*0.90f;
            b.add(MeshShape::Cone,{0,y,-10.5f},{w,2.0f,w},{0.045f,0.20f,0.10f},Surface::Leaves);
            b.add(MeshShape::Cone,{0,y+0.58f,-10.5f},{w*0.63f,0.85f,w*0.63f},snow,Surface::Snow);
            for(int j=0;j<8;++j) {
                float a=j*glm::two_pi<float>()/8+layer*0.5f;
                glm::vec3 p(std::cos(a)*w*0.39f,y-0.55f,-10.5f+std::sin(a)*w*0.39f);
                b.ball(p,{0.20f,0.20f,0.20f},j%2?glm::vec3(0.78f,0.10f,0.08f):glm::vec3(0.93f,0.62f,0.10f),Surface::Metal);
            }
        }
        b.add(MeshShape::Sphere,{0,4.65f,-10.5f},{0.38f,0.55f,0.38f},{1,0.68f,0.16f},Surface::Metal,{0,0,0},1.6f);
        for(int i=0;i<6;++i) {
            glm::vec3 p(-2.5f+i*1.0f,-1.07f,-8.6f);
            b.box(p,{0.75f,0.8f,0.70f},i%2?glm::vec3(0.65f,0.07f,0.06f):glm::vec3(0.09f,0.26f,0.50f),Surface::Plastic);
            b.box(p+glm::vec3(0,0.415f,0),{0.12f,0.025f,0.72f},{0.95f,0.73f,0.24f},Surface::Metal);
        }
    }
    return b.items;
}
std::vector<SceneObject> Environment::animated(Location l,float time,bool high) {
    Builder b;
    if(l==Location::Winter)for(int i=0;i<(high?90:30);++i) {
        float x=float((i*73)%420)/10-21,z=float((i*97)%420)/10-21;
        float y=9-std::fmod(time*(0.35f+0.03f*(i%4))+i*0.19f,10.3f);
        b.add(MeshShape::Sphere,{x+0.2f*std::sin(time+i),y,z},{0.045f,0.045f,0.045f},
            {0.9f,0.95f,1},Surface::Snow,{0,0,0},0,false);
    }
    if(l==Location::Winter)for(int i=0;i<35;++i) {
        float t=float(i)/34,a=t*18.85f+time*0.03f,rad=2.3f*(1-t)+0.12f;
        b.add(MeshShape::Sphere,{std::cos(a)*rad,-0.7f+t*5,-10.5f+std::sin(a)*rad},
            {0.075f,0.075f,0.075f},{1.0f,0.58f,0.13f},Surface::Plastic,{0,0,0},2.5f,false);
    }
    return b.items;
}
