#include "Environment.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
static void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
int main(){try{
 SunPosition sun;auto initial=sun.direction();
 require(std::abs(glm::length(initial)-1)<0.0001f,"Sun is not normalized");
 sun.move(1,1,0);require(glm::length(sun.direction()-initial)<0.000001f,"Zero dt moved sun");
 sun.move(1,0,0.1f);require(glm::length(sun.direction()-initial)>0.01f,"Azimuth does not work");
 for(int i=0;i<500;++i)sun.move(0,1,0.1f);
 require(std::abs(sun.elevationDegrees()-82)<0.001f,"Upper bound failed");
 for(int i=0;i<500;++i)sun.move(0,-1,0.1f);
 require(std::abs(sun.elevationDegrees()-8)<0.001f,"Sun crossed horizon");
 sun.reset();require(glm::length(sun.direction()-initial)<0.000001f,"Sun reset failed");
 for(auto l:{Location::Park,Location::Beach,Location::Winter}){
  for(bool high:{false,true}){
   auto scenery=Environment::build(l,high);require(!scenery.empty(),"Empty location");
   auto moving=Environment::animated(l,123.0f,high);scenery.insert(scenery.end(),moving.begin(),moving.end());
   for(const auto& o:scenery){
    require(std::isfinite(o.center.x)&&std::isfinite(o.center.y)&&std::isfinite(o.center.z),"Non-finite scene");
    require(o.size.x>0&&o.size.y>0&&o.size.z>0,"Invalid object size");
    require(static_cast<int>(o.shape)>=0&&static_cast<int>(o.shape)<4,"Invalid mesh");
   }
  }
 }
 require(Environment::build(Location::Park,true).size()>Environment::build(Location::Park,false).size(),"Quality toggle ineffective");
 std::cout<<"Environment and fixed sun tests passed\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
