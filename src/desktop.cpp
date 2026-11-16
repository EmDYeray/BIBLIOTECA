#include "desktop.hpp"
#include "render.hpp"
#include "ui.hpp"
#include <chrono>
#include <stdexcept>

namespace biblioteca {
Desktop::Desktop(Size size,bool hidden){
        if(SDL_Init(sdl::video)!=0)throw std::runtime_error(SDL_GetError());
        SDL_GL_SetAttribute(sdl::major,3);SDL_GL_SetAttribute(sdl::minor,3);SDL_GL_SetAttribute(sdl::profile,1);
        SDL_GL_SetAttribute(sdl::depthSize,24);SDL_GL_SetAttribute(sdl::doubleBuffer,1);
        SDL_GL_SetAttribute(sdl::msBuffers,1);SDL_GL_SetAttribute(sdl::msSamples,2);
        const unsigned flags=sdl::openGL|(hidden?sdl::hidden:sdl::resizable|sdl::highDpi);
        auto create=[&](){return SDL_CreateWindow("Biblioteca",sdl::centered,sdl::centered,size.w,size.h,flags);};
        window=create();
        if(window)context=SDL_GL_CreateContext(window);
        if(!context){
            if(window)SDL_DestroyWindow(window);
            SDL_GL_SetAttribute(sdl::msBuffers,0);SDL_GL_SetAttribute(sdl::msSamples,0);
            window=create();if(window)context=SDL_GL_CreateContext(window);
        }
        if(!context){
            const std::string failure=SDL_GetError();
            if(window)SDL_DestroyWindow(window);
            SDL_Quit();throw std::runtime_error(failure);
        }
#ifdef _WIN32
        glewExperimental=GL_TRUE;
        if(glewInit()!=GLEW_OK)throw std::runtime_error("No se pudo inicializar OpenGL 3.3");
        glGetError();
#endif
        SDL_SetWindowMinimumSize(window,720,490);
        SDL_StopTextInput();
        SDL_GL_SetSwapInterval(hidden?0:1);
        glEnable(gl::multisample);
    }
Desktop::~Desktop(){SDL_GL_DeleteContext(context);SDL_DestroyWindow(window);SDL_Quit();}

namespace {
using Clock=std::chrono::steady_clock;
class Cursors {
    SDL_Cursor* arrow=SDL_CreateSystemCursor(sdl::arrowCursor);
    SDL_Cursor* hand=SDL_CreateSystemCursor(sdl::handCursor);
    SDL_Cursor* grab=SDL_CreateSystemCursor(sdl::grabCursor);
    int current=-1;
public:
    ~Cursors(){if(arrow)SDL_FreeCursor(arrow);if(hand)SDL_FreeCursor(hand);if(grab)SDL_FreeCursor(grab);}
    void set(int next){
        if(next==current)return;
        current=next;auto* pointer=next==sdl::handCursor?hand:next==sdl::grabCursor?grab:arrow;
        if(pointer)SDL_SetCursor(pointer);
    }
};

} // namespace

int interactive(){
    const Size initial{1280,800};
    Desktop desktop(initial,false);
    Renderer renderer;Cursors cursors;
    Interface ui(initial);
    Size logical=initial;
    std::string hud;
    const auto start=Clock::now();
    auto last=start,lastFrame=start;
    constexpr auto idleFrame=std::chrono::milliseconds(33);
    SDL_Event event{};
    while(!ui.quit){
        if(!ui.dirty && !ui.walking() && !ui.lib.animating()){
            const auto left=std::chrono::ceil<std::chrono::milliseconds>(idleFrame-(Clock::now()-lastFrame));
            if(left.count()>0 && SDL_WaitEventTimeout(&event,int(left.count())))ui.event(event,desktop.window);
        }
        while(SDL_PollEvent(&event))ui.event(event,desktop.window);
        const auto now=Clock::now();
        const float elapsed=std::chrono::duration<float>(now-last).count();last=now;
        const float sceneSeconds=ui.sceneTime(elapsed);
        ui.step(std::min(.09f,elapsed));
        if(ui.quit)break;
        Size size;SDL_GetWindowSize(desktop.window,&size.w,&size.h);
        if(size.w!=logical.w || size.h!=logical.h){logical=size;ui.resize(size);}
        cursors.set(ui.pointerKind());
        const auto signature=ui.signature();
        if(!ui.dirty && hud==signature && now-lastFrame<idleFrame)continue;
        SDL_GL_GetDrawableSize(desktop.window,&size.w,&size.h);
        if(size.w<=0 || size.h<=0){lastFrame=now;continue;}
        renderer.draw(ui.lib,size,sceneSeconds);
        if(hud!=signature){
            Paint p(logical);ui.paint(p);renderer.hud(&p);hud=signature;
        }else renderer.hud(nullptr);
        SDL_GL_SwapWindow(desktop.window);ui.dirty=false;lastFrame=now;
        if(ui.lib.animating() && elapsed<.007f){
            SDL_Event pending{};
            if(SDL_WaitEventTimeout(&pending,5))ui.event(pending,desktop.window);
        }
    }
    return 0;
}

}
