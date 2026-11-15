#include "render.hpp"
#include "ui.hpp"
#include <SDL_system.h>
#include <memory>
#include <stdexcept>

namespace {
using namespace biblioteca;

struct App {
    SDL_Window* window;
    Size logical;
    Interface ui;
    Renderer renderer;
    std::string hud;
    Uint64 last=SDL_GetPerformanceCounter();

    App(SDL_Window* window,Size size):window(window),logical(size),ui(size){}

    void draw() {
        SDL_Event event{};
        while(SDL_PollEvent(&event))ui.event(event,window);
        if(ui.quit){
            SDL_iPhoneSetAnimationCallback(window,1,nullptr,nullptr);
            return;
        }

        const Uint64 now=SDL_GetPerformanceCounter();
        const float elapsed=std::min(.09f,float(now-last)/float(SDL_GetPerformanceFrequency()));
        last=now;
        const float sceneTime=ui.sceneTime(elapsed);
        Size size;
        SDL_GetWindowSize(window,&size.w,&size.h);
        if(size.w<=0 || size.h<=0)return;
        if(size.w!=logical.w || size.h!=logical.h){
            logical=size;
            ui.resize(size);
            hud.clear();
        }
        ui.step(elapsed);
        SDL_GL_GetDrawableSize(window,&size.w,&size.h);
        if(size.w<=0 || size.h<=0)return;
        renderer.draw(ui.lib,size,sceneTime);
        const auto signature=ui.signature();
        if(hud!=signature){
            Paint image(logical);
            ui.paint(image);
            renderer.hud(&image);
            hud=signature;
        }else renderer.hud(nullptr);
        SDL_GL_SwapWindow(window);
        ui.dirty=false;
    }
};

// SDL hands control back to UIKit after SDL_main. The callback and its
// GL context therefore live for the lifetime of the application.
std::unique_ptr<App> running;

void frame(void* state) {
    auto& app=*static_cast<App*>(state);
    try {
        app.draw();
    }catch(const std::exception& error){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",error.what());
        SDL_iPhoneSetAnimationCallback(app.window,1,nullptr,nullptr);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Biblioteca",error.what(),app.window);
    }
}
}

int main(int,char**) {
    try {
        if(SDL_Init(SDL_INIT_VIDEO)!=0)throw std::runtime_error(SDL_GetError());
        SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,"1");
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING,0);
        const auto flags=SDL_WINDOW_OPENGL|SDL_WINDOW_FULLSCREEN|SDL_WINDOW_ALLOW_HIGHDPI;
        auto* window=SDL_CreateWindow("Biblioteca",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1024,768,flags);
        if(!window || !SDL_GL_CreateContext(window))throw std::runtime_error(SDL_GetError());
        SDL_StopTextInput();
        Size size;
        SDL_GetWindowSize(window,&size.w,&size.h);
        auto app=std::make_unique<App>(window,size);
        if(SDL_iPhoneSetAnimationCallback(window,1,frame,app.get())!=0)
            throw std::runtime_error(SDL_GetError());
        running=std::move(app);
        return 0;
    }catch(const std::exception& error){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",error.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Biblioteca",error.what(),nullptr);
        return 1;
    }
}
