#pragma once
#include "library.hpp"

namespace biblioteca {
constexpr float roomHalfWidth=17.05f, roomBack=-2.40f, roomFront=15.20f;
constexpr float roomFloor=-2.26f, roomCeiling=6.64f, personRadius=.32f, walkingEye=-.46f;
constexpr float adultRight=36.80f,adultShelfX=27.0f,doorZ=7.f,doorHalf=1.63f;
constexpr std::array<Vec,4> lanterns{{{22.5f,2.43f,3.7f},{31.5f,2.43f,3.7f},
                                    {22.5f,2.43f,10.6f},{31.5f,2.43f,10.6f}}};
constexpr Vec hearthAt{0,roomFloor,7.0f};
constexpr float hearthRadius=1.31f;
constexpr Vec referenceLecternAt{roomHalfWidth+3.12f,roomFloor,doorZ-1.83f};
Bounds referenceLecternBounds();
constexpr float caseDepth=1.10f,caseTop=4.50f;
enum class CaseWall {Stories,Left,Right,Rear};
struct DisplayCase {Vec at;float width;CaseWall wall;unsigned seed;};
constexpr size_t sideCaseCount=4,rearCaseCount=6;
constexpr size_t childrenDisplayCases=3+2*sideCaseCount+rearCaseCount;
constexpr size_t displayCaseCount=childrenDisplayCases+10;
const std::array<DisplayCase,displayCaseCount>& displayCases();
Bounds displayCaseBounds(const DisplayCase& item);
float displayCaseHeading(CaseWall wall);
bool splitCase(const DisplayCase& item);
constexpr float displayLevel(int row){return -2.10f+row*1.074f;}
std::vector<Bounds> displayCaseBoards(const DisplayCase& item);
enum class Furnishing {Table,Chair,Bench};
struct Furniture {Furnishing kind; Vec at; float face=1;};
constexpr size_t furnitureCount=14;
const std::array<Furniture,furnitureCount>& furniture();
Bounds furnitureBounds(const Furniture& item);
std::array<Bounds,3> dividerWalls();
Bounds openDoorLeaf();
const std::vector<Bounds>& roomOccluders();
bool walkable(Vec p);
Vec walkStep(Vec start,Vec displacement);
std::vector<Vec> routeInside(Vec from,Vec to);
}
