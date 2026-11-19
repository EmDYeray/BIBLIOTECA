#pragma once

#include "geometry.hpp"
#include <filesystem>

namespace biblioteca {

enum class ContentKind { Shelf,Fire,Lantern,Table,Chair,Bench,Book,Box,Spawn,Portal };

struct ContentRoom {
    std::string id,name;
    Vec origin{},size{10,0,10};
    float height=5;
    bool builtin=false;
    std::string floor="wood",walls="plaster",ceiling="plaster";
};

struct ContentObject {
    std::string id,name;
    ContentKind kind=ContentKind::Box;
    Vec position{},rotation{},scale{1,1,1};
    std::string color="#8b7059",material="wood";
    bool solid=true,animated=false,interactive=false;
    int book=-1;
};

struct ContentScene {
    int schema=2;
    std::vector<ContentRoom> rooms;
    std::vector<ContentObject> objects;
    std::vector<std::string> warnings;
};

ContentScene loadContentScene(const std::filesystem::path& path);
const ContentScene& contentScene();
MeshData buildContentSceneMesh(const ContentScene& scene);
std::vector<Bounds> buildContentSceneColliders(const ContentScene& scene);
bool contentSceneContains(const ContentScene& scene,Vec point);
MeshData contentSceneMesh();
const std::vector<Bounds>& contentSceneColliders();
bool contentSceneContains(Vec point);

}
