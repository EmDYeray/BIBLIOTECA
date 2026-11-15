#pragma once
#include "library.hpp"
#include "native.hpp"

namespace biblioteca {
class Desktop {
public:
    SDL_Window* window=nullptr;
    SDL_GLContext context=nullptr;
    Desktop(Size size,bool hidden);
    ~Desktop();
    Desktop(const Desktop&)=delete;
    Desktop& operator=(const Desktop&)=delete;
};
int interactive();
}
