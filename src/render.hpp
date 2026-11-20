#pragma once
#include "library.hpp"
#include <memory>
namespace biblioteca {
class Paint;
struct RenderStats {int calls=0,triangles=0;};
class Renderer {
    struct Data;
    std::unique_ptr<Data> data;
public:
    Renderer();
    ~Renderer();
    void draw(const Library& lib,Size size,float seconds=0,float sceneScale=1.f);
    void hud(const Paint* image);
    RenderStats stats() const;
};
}
