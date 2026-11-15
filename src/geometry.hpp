#pragma once
#include "room_layout.hpp"
#include "paint.hpp"

namespace biblioteca {
enum Material {Wood,Brass,Paper,Leather,Wall,Floor,Enamel,Card,Metal=12,SolarCell,Lamp,Textile,Rubber,Planet,
               LightStrip,Ember,Tile,PaintedWood,Frosted};
constexpr int printedFront=10, printedBack=11;
struct Vertex { Vec p,n,c; float u,v,mat; };
using MeshData=std::vector<Vertex>;
inline constexpr Vec white{1,1,1};
Vec tint(Color c);
void quad(MeshData& out,const std::array<Vertex,4>& q);
void box(MeshData& out,Vec center,Vec extent,Vec color,Material mat,float bevel=.025f,int printAxis=-1,int printSide=1);
struct RoomSection {MeshData mesh;};
constexpr size_t roomSectionCount=2+displayCaseCount+3;
std::array<RoomSection,roomSectionCount> room();
MeshData turningSheet(float t);
}
