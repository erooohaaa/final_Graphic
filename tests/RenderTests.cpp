#include <glad/gl.h>
#include "App.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
extern "C" {
void* eglGetProcAddress(const char*);
unsigned int eglInitialize(void*,int*,int*);
unsigned int eglBindAPI(unsigned int);
unsigned int eglChooseConfig(void*,const int*,void**,int,int*);
void* eglCreatePbufferSurface(void*,void*,const int*);
void* eglCreateContext(void*,void*,void*,const int*);
unsigned int eglMakeCurrent(void*,void*,void*,void*);
unsigned int eglDestroyContext(void*,void*);
unsigned int eglDestroySurface(void*,void*);
unsigned int eglTerminate(void*);
}
static void require(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
static std::vector<unsigned char> pixels(int w=1280,int h=800){
 std::vector<unsigned char> p(w*h*3);glPixelStorei(GL_PACK_ALIGNMENT,1);glFinish();
 glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,p.data());require(glGetError()==GL_NO_ERROR,"OpenGL error");return p;
}
static void snap(const char* path,int w=1280,int h=800){
 auto p=pixels(w,h);std::ofstream f(path,std::ios::binary);f<<"P6\n"<<w<<" "<<h<<"\n255\n";
 for(int y=h-1;y>=0;--y)f.write((char*)p.data()+y*w*3,w*3);
}
int main(int argc,char** argv){try{
 using DisplayProc=void*(*)(unsigned int,void*,const int*);
 auto getDisplay=reinterpret_cast<DisplayProc>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
 require(getDisplay!=nullptr,"No surfaceless EGL");
 void* d=getDisplay(0x31DD,nullptr,nullptr);int ma,mi;
 require(eglInitialize(d,&ma,&mi),"EGL initialize");require(eglBindAPI(0x30A2),"OpenGL binding");
 int attrs[]={0x3033,1,0x3040,8,0x3024,8,0x3023,8,0x3022,8,0x3025,24,0x3038};
 void* config;int count;require(eglChooseConfig(d,attrs,&config,1,&count)&&count,"EGL config");
 int size[]={0x3057,1280,0x3056,800,0x3038};void* surface=eglCreatePbufferSurface(d,config,size);
 int ctxattr[]={0x3098,3,0x30FB,3,0x30FD,1,0x3038};
 void* ctx=eglCreateContext(d,config,nullptr,ctxattr);require(ctx&&surface,"Context creation");
 require(eglMakeCurrent(d,surface,surface,ctx),"EGL current");require(gladLoadGL((GLADloadfunc)eglGetProcAddress),"GLAD load");
 std::cout<<glGetString(GL_VERSION)<<" / "<<glGetString(GL_RENDERER)<<'\n';
 std::filesystem::path shaders=argc>1?argv[1]:"assets/shaders";
 std::filesystem::create_directories("render-check");
 {
  App app(shaders,{},1234);app.resize(1280,800);app.startDemo();app.update(0.016f);app.render();
  snap("render-check/park.ppm");auto lit=pixels();
  app.toggleShadows();app.render();auto unshadowed=pixels();snap("render-check/no-shadows.ppm");
  int changed=0;
  for(int y=100;y<650;++y)for(int x=40;x<900;++x){
   int i=(y*1280+x)*3;
   if(std::abs(int(lit[i])-int(unshadowed[i]))+std::abs(int(lit[i+1])-int(unshadowed[i+1]))>15)++changed;
  }
  require(changed>100,"Shadow toggle has no visible effect");std::cout<<"Shadow comparison changed "<<changed<<" scene pixels\n";
  app.toggleShadows();
  auto fixed=app.sunDirection();app.orbit(60,-10);app.zoom(-1);
  require(glm::length(app.sunDirection()-fixed)<0.000001f,"Camera moved sunlight");app.resetCamera();
  auto pos=app.boardCellScreen(7,7);app.pointerMove(pos.x,pos.y);app.pointerDown();
  require(app.game().score()==120,"Demo picking/clear regression");
  app.cycleLocation();app.update(0.016f);app.render();snap("render-check/beach.ppm");
  require(app.game().score()==120&&glm::length(app.sunDirection()-fixed)<0.000001f,"Theme reset game or sunlight");
  app.cycleLocation();app.update(0.016f);app.render();snap("render-check/winter.ppm");
  app.togglePause();app.update(0.1f);app.render();snap("render-check/paused.ppm");app.togglePause();
  app.toggleHelp();app.render();snap("render-check/help.ppm");app.toggleHelp();
  app.toggleQuality();app.moveLight(1,1,0,0.1f);require(glm::length(app.sunDirection()-fixed)>0.01f,"Sun controls ineffective");
  app.toggleLightMotion();require(glm::length(app.sunDirection()-fixed)<0.000001f,"Reset sun failed");
  app.resize(960,600);app.orbit(25,10);app.render();snap("render-check/resized.ppm",960,600);
  require(glGetError()==GL_NO_ERROR,"Final app GL error");
 }
 // Infinite sky has no translational parallax; the sun is not scene geometry.
 {
  Renderer renderer(shaders);SunPosition sun;auto direction=sun.direction();
  glViewport(0,0,1280,800);glDisable(GL_SCISSOR_TEST);glClear(GL_DEPTH_BUFFER_BIT);
  auto projection=glm::perspective(glm::radians(45.0f),1.6f,0.1f,220.0f);
  auto skyRender=[&](glm::vec3 eye){
   renderer.configure(Environment::skyColor(Location::Park),0,false,true);
   renderer.begin(glm::lookAt(eye,eye+direction,glm::vec3(0,1,0)),projection,eye,direction);renderer.end(true);
  };
  skyRender({0,2,0});auto first=pixels();snap("render-check/sky-sun.ppm");
  skyRender({8,5,-6});auto second=pixels();long difference=0;
  for(std::size_t i=0;i<first.size();++i)difference+=std::abs(int(first[i])-int(second[i]));
  require(double(difference)/first.size()<0.15,"Sky sun has translational parallax");
  std::cout<<"Infinite sky translation difference: "<<double(difference)/first.size()<<'\n';
 }
 eglMakeCurrent(d,nullptr,nullptr,nullptr);eglDestroyContext(d,ctx);eglDestroySurface(d,surface);eglTerminate(d);
 std::cout<<"Render tests passed\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
