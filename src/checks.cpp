#include "checks.hpp"
#include "desktop.hpp"
#include "room_layout.hpp"
#include "popup.hpp"
#include "stories.hpp"
#include "render.hpp"
#include "references.hpp"
#include "ui.hpp"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <span>
#include <stdexcept>

namespace biblioteca {
namespace {
using Clock=std::chrono::steady_clock;
constexpr Size proofSize{1280,800};
constexpr float tick=1.f/120;

void expect(bool value,std::string_view why){if(!value)throw std::runtime_error(std::string(why));}
template<class Step> void advance(Step& x,float time){for(int i=0;i<int(std::ceil(time/tick));++i)x.update(tick);}
void advance(Interface& x,float time){for(int i=0;i<int(std::ceil(time/tick));++i)x.step(tick);}
void until(Library& lib,Stage target){
    for(int i=0;i<700 && lib.stage()!=target;++i)lib.update(tick);
    expect(lib.stage()==target,"Animation did not reach the expected stage");
}
void until(Interface& ui,Stage target){
    for(int i=0;i<2400 && ui.lib.stage()!=target;++i)ui.step(tick);
    expect(ui.lib.stage()==target,"Interaction did not reach the expected stage. Stage "+
           std::to_string(int(ui.lib.stage()))+", x="+std::to_string(ui.lib.eye().x)+
           ", z="+std::to_string(ui.lib.eye().z));
}
} // namespace

void geometryTest(){
    Library walking;
    const Vec initial=walking.eye(),oldDirection=walking.forward();
    expect(walkable(initial),"Camera starts inside furniture");
    walking.look(370,150);
    expect(dot(oldDirection,walking.forward())<.5f,"Mouse input does not change camera direction");
    walking.walk(2.0f,0);
    expect(walkable(walking.eye()) && walking.eye().y==walkingEye &&
           dot(walking.eye()-initial,walking.eye()-initial)>.5f,"Camera cannot move through the room");
    for(const Vec delta:{Vec{200,0,0},Vec{-200,0,0},Vec{0,0,200},Vec{0,0,-200},Vec{120,0,-90}}){
        const Vec p=walkStep({-2.65f,walkingEye,6.45f},delta);
        expect(walkable(p),"Camera crossed the room boundary");
    }
    const Vec approachTable=walkStep({5.85f,walkingEye,6.70f},{0,0,20});
    expect(approachTable.z<7.6f,"Camera intersected the chair near the table");
    const Vec hearthLeft{-3.4f,walkingEye,hearthAt.z},hearthRight{3.4f,walkingEye,hearthAt.z};
    const Vec approachHearth=walkStep(hearthLeft,hearthRight-hearthLeft);
    expect(approachHearth.x<=-hearthRadius-personRadius+.001f &&
           approachHearth.x>-hearthRadius-personRadius-.10f,"Camera did not stop at the hearth ring");
    const auto aroundHearth=routeInside(hearthLeft,hearthRight);
    expect(aroundHearth.size()>1 && walkable(aroundHearth.back()),"No automatic path around the hearth");
    const Vec beforeShelf=walkStep({wingX(0),walkingEye,4.25f},{0,0,-30});
    expect(beforeShelf.z>1.52f,"Camera entered the shelf's book-extraction area");
    const Vec insideDoor{roomHalfWidth-1.65f,walkingEye,doorZ};
    const Vec beyondDoor=walkStep(insideDoor,{3.55f,0,0});
    expect(beyondDoor.x>roomHalfWidth+1.f && walkable(beyondDoor),"Door prevents crossing between rooms");
    const Vec againstWall=walkStep({roomHalfWidth-2.9f,walkingEye,doorZ+2.6f},{6.9f,0,0});
    expect(againstWall.x<roomHalfWidth,"Camera crossed outside the doorway");
    Library transfer;
    expect(!transfer.visit(originalCount) && transfer.enter(Section::Adult),"Catalog mixed the room collections");
    for(int i=0;i<980 && transfer.stage()==Stage::Traveling;++i){
        transfer.update(tick);
        expect(walkable(transfer.eye()),"Path intersected a wall or furniture");
    }
    expect(transfer.stage()==Stage::Idle && transfer.section()==Section::Adult &&
           !transfer.visit(0),"Failed to reach the teen and adult room");
    expect(transfer.enter(Section::Children),"Cannot return to the children's room");
    until(transfer,Stage::Idle);
    expect(walking.travel({5.85f,walkingEye,5.38f},{5.85f,.2f,.5f}),"Catalog cannot find a route");
    for(int i=0;i<850 && walking.stage()!=Stage::Idle;++i){
        walking.update(tick);
        expect(walkable(walking.eye()),"Automatic route intersects furniture");
    }
    expect(walking.stage()==Stage::Idle && std::abs(walking.eye().z-5.38f)<.02f,"Failed to reach the aisle");
    int quietBooks=0;
    const auto roomMeshes=room();
    for(const auto& part:roomMeshes)expect(!part.mesh.empty(),"Missing a room's 3D section");
    const auto& cases=displayCases();
    for(size_t i=0;i<cases.size();++i){
        const auto& item=cases[i];
        const auto bounds=displayCaseBounds(item);
        for(const auto& vertex:roomMeshes[i+1].mesh)
            for(int axis=0;axis<3;++axis)
                expect(vertex.p[axis]>=bounds.min[axis]-.001f && vertex.p[axis]<=bounds.max[axis]+.001f,
                       "Bookshelf extends beyond its collision bounds");
        if(item.wall==CaseWall::Stories)continue;
        const Vec inward=item.wall==CaseWall::Left?Vec{1,0,0}
            :item.wall==CaseWall::Right?Vec{-1,0,0}:Vec{0,0,-1};
        const Vec center{item.at.x,walkingEye,item.at.z};
        const Vec start=center+inward*(caseDepth*.5f+1.03f);
        expect(walkable(start),"Cannot walk in front of a perimeter shelf");
        const Vec stop=walkStep(start,(center-start)*2.f);
        const float gap=dot(stop-center,inward)-caseDepth*.5f;
        expect(gap>=personRadius-.001f && gap<personRadius+.10f,
               "Camera intersects a perimeter shelf");
    }
    for(int id=0;id<originalCount;++id){
        if(!panoramicBook(id))continue;
        quietBooks+=wordlessBinding(id);
        for(int spread=0;spread<spreads;++spread){
            const auto& content=chapter(id,spread);
            expect(!content.title.empty() && !content.left.empty() && !content.right.empty(),
                   "Missing story text");
        }
    }
    expect(quietBooks>0 && quietBooks<childrenCount,
           "Missing books with wordless bindings");
    for(int id=0;id<bookCount;++id)
        for(int page=0;page<spreadCount(id)*2;++page){
            const auto prose=pageText(id,page);
            if(bookSection(id)==Section::Adult)
                expect(prose.size()>200,"Missing a long reading");
            else if(id>=firstImport)
                expect(prose.size()>85 && prose.size()<340,"Unexpected length in adapted reading "+
                       std::to_string(id)+" page "+std::to_string(page+1)+": "+std::to_string(prose.size()));
            else expect(!prose.empty() && prose.size()<190,"Children's text does not fit beside the illustration");
        }
    for(int id=0;id<bookCount;++id){
        std::array<bool,popupCount> used{};
        for(int spread=0;spread<spreadCount(id);++spread){
            const auto index=popupIndex(id,spread);
            expect(!used.at(index),"Repeated illustration in "+std::string(books()[id].title));
            used[index]=true;
        }
    }
    expect(motionStrength(1,0,true)==1.f && motionStrength(1,popTurnStart,true)==0.f &&
           motionStrength(1,popTurnEnd,true)==0.f && motionStrength(1,1,true)==1.f,
           "Model keeps flying during a page turn");
    for(const auto figure:{Figure::Mars,Figure::Telescope,Figure::Saturn,Figure::PioneerBelt,Figure::ApolloDrop}){
        const auto scene=loadPopup(assetsPath()/"popups"/popupAssets()[int(figure)].file);
        expect(!scene.actors.empty(),"Scene has no animated elements");
        for(const auto& actor:scene.actors){
            const auto fixed=animatePopup(actor,0,3.f);
            bool moves=false;
            for(const auto& vertex:actor.mesh){
                const Vec collapsed=fixed.point(vertex.p)-vertex.p;
                expect(dot(collapsed,collapsed)<1e-7f,"Paper cutout loses its folded pose");
                for(float phase:{.6f,1.4f,2.8f,4.8f}){
                    const Vec delta=animatePopup(actor,1,phase).point(vertex.p)-animatePopup(actor,1,0).point(vertex.p);
                    moves=moves || dot(delta,delta)>.00005f;
                }
            }
            expect(moves,"Animation transform does not move the object");
        }
    }
    const auto launchScene=loadPopup(assetsPath()/"popups"/"cohete.obj");
    bool launchFound=false;
    for(const auto& actor:launchScene.actors)if(actor.motion==PopupMotion::Launch){
        launchFound=true;
        expect(!actor.mesh.empty(),"Rocket launch mesh is missing");
    }
    expect(launchFound,"Rocket launch animation is missing");
    const auto assembly=loadPopup(assetsPath()/"popups"/"orbita-ensamble.obj");
    int assemblyLaunches=0,assemblySatellites=0;
    for(const auto& actor:assembly.actors){
        if(actor.motion==PopupMotion::Launch){
            ++assemblyLaunches;
            for(const auto& vertex:actor.mesh)
                expect(vertex.p.x<-.13f,"Rocket trail contains support geometry");
        }else if(actor.motion==PopupMotion::Flight){
            ++assemblySatellites;
            for(const auto& vertex:actor.mesh)
                expect(vertex.p.x>0 && vertex.p.y>.07f,"Satellite trail contains support geometry");
        }
    }
    expect(assemblyLaunches==1 && assemblySatellites==1,"Assembly platform is missing trails");

    Library launchReader;
    const int pioneer=firstImport+4;
    expect(launchReader.visit(pioneer),"Cannot open Pioneer reading");
    until(launchReader,Stage::Reading);
    expect(launchReader.popupClock()==0,"Launch clock started while book was closed");
    advance(launchReader,launchDuration+tick);
    expect(launchReader.popupClock()>launchDuration,"Launch clock stopped before the trail faded");
    advance(launchReader,trailFadeDuration+tick);
    expect(launchReader.popupClock()==launchSequenceDuration,"Launch clock continued after the trail faded");
    expect(launchReader.turn(1),"Cannot turn forward from the rocket page");
    advance(launchReader,turnDuration+tick);
    expect(launchReader.turn(-1),"Cannot turn back to the rocket page");
    advance(launchReader,turnDuration+tick);
    expect(launchReader.spread()==0 && launchReader.popupClock()<.03f,
           "Launch does not restart when revisiting its scene");
    launchReader.setMotionEnabled(false);
    const float paused=launchReader.popupClock();
    advance(launchReader,.4f);
    expect(launchReader.popupClock()==paused,"Launch continued while animation was paused");
    expect(dynamicPopup(PopupMotion::Launch) && dynamicPopup(PopupMotion::Flight) &&
           dynamicPopup(PopupMotion::Drift) && dynamicPopup(PopupMotion::Drop) &&
           dynamicPopup(PopupMotion::Spin) && dynamicPopup(PopupMotion::Scan) &&
           dynamicPopup(PopupMotion::Roll) && dynamicPopup(PopupMotion::Slide),
           "An articulated object lost its motion");
    for(const auto& asset:popupAssets()){
        const auto scene=loadPopup(assetsPath()/"popups"/asset.file);
        const bool hasLight=std::any_of(scene.resting.begin(),scene.resting.end(),
                                      [](const auto& vertex){return vertex.mat==float(Lamp);});
        expect(!scene.actors.empty() || hasLight,"Unanimated scene: "+std::string(asset.file));
        const auto cutouts=foldablePanels(scene);
        for(bool compact:{false,true})for(int side=0;side<2;++side){
            const auto& panel=cutouts[side];
            expect(!panel.empty(),"Missing paper cutout in "+std::string(asset.file));
            const float sign=side==0?-1.f:1.f;
            const auto flat=paperPanel(0,compact,side);
            float highest=0;
            for(const auto& vertex:panel){
                const Vec v=flat.point(vertex.p);
                expect(v.x*sign>=pageInner && v.x*sign<=pageOuter && std::abs(v.y)<=pageHeight/2,
                       "Folded paper cutout extends outside the page: "+std::string(asset.file));
                expect(v.z>pageBlock+.001f && v.z<pageBlock+hingeGap-.001f,
                       "Folded cutout does not fit between pages");
                highest=std::max(highest,paperPanel(1,compact,side).point(vertex.p).z);
            }
            expect(highest>pageBlock+.20f,"Missing depth in "+std::string(asset.file));
            for(float amount:{.10f,.25f,.48f,.62f,.75f,.90f,1.f}){
                const auto panelPose=paperPanel(amount,compact,side);
                for(Vec axis:{Vec{1,0,0},Vec{0,1,0}}){
                    const Vec before=flat.point(axis,true),after=panelPose.point(axis,true);
                    expect(std::abs(dot(before,before)-dot(after,after))<.0001f,
                           "Cutout changes scale while folding");
                }
                for(const auto& vertex:panel)
                    expect(panelPose.point(vertex.p).z>=pageBlock+.001f,
                           "Cutout passes through its page: "+std::string(asset.file));
            }
            for(float t:{0.f,.18f,.45f,.69f,.91f,1.f}){
                const auto attached=attachToSheet(panel,t,compact,side);
                for(size_t i=0;i<attached.size();++i){
                    const auto v=flat.point(panel[i].p);
                    const float radial=v.x*sign-pageInner;
                    const Vec anchor=leafPoint(radial,v.y,t);
                    const Vec tangent=unit(leafPoint(radial+.0005f,0,t)-leafPoint(radial-.0005f,0,t));
                    const float height=dot(attached[i].p-anchor,cross(tangent,{0,1,0}))*sign;
                    expect(std::abs(height-(v.z-pageBlock))<.0001f,
                           "Cutout detaches from the bent page");
                }
            }
        }
    }
    for(int direction:{-1,1}){
        const int start=direction<0?1:0,end=1-start;
        const auto begin=readingState(start,end,0,1),down=readingState(start,end,popTurnStart,1),
                   turn=readingState(start,end,.5f,1),land=readingState(start,end,popTurnEnd,1),
                   done=readingState(start,end,1,1);
        expect(begin.deploy==1 && !begin.turning && down.deploy==0,"Pop-up does not fold before turning");
        expect(turn.turning && turn.deploy==0 && turn.attachedBookSpread==0,"Figure does not follow the turning page");
        expect(!land.turning && land.deploy==0 && land.restingBookSpread==end && done.deploy==1,
               "New figure appears before the page settles");
    }
    expect(readingState(0,0,1,.58f).deploy==0,"Figure remains raised when the book closes");
    for(const auto& slot:positions()){
        const bool child=bookSection(slot.id)==Section::Children;
        const auto shelf=Shelf{child?wingX(slot.wing):adultShelfX};
        const auto localSolids=shelf.solids();
        const auto outerSolids=slot.cabinet<0?std::vector<Bounds>{}:displayCaseBoards(displayCases()[slot.cabinet]);
        const float level=slot.cabinet<0?Shelf::levels[slot.row]:displayLevel(slot.row);
        expect(std::abs(slot.bounds().min.y-level)<.0001f,"Book does not rest on its board");
        expect(books()[slot.id].width<.40f,"Closed book is too thick");
        const auto solids=slot.cabinet<0?std::span<const Bounds>(localSolids):std::span<const Bounds>(outerSolids);
        for(int i=0;i<=32;++i){
            const auto current=slot.bounds((extractedZ-slotZ)*i/32.f);
            for(const auto& solid:solids)expect(!current.overlaps(solid),"Book intersects a board");
            for(const auto& neighbor:positions())if(neighbor.id!=slot.id)
                expect(!current.overlaps(neighbor.bounds()) && !current.overlaps(neighbor.bounds(.09f)),
                       "Book intersects its neighbor");
        }
        Library lib;
        if(!child){expect(lib.enter(Section::Adult),"Cannot enter the adult room");until(lib,Stage::Idle);}
        const bool target=lib.travel(slot.approachPoint(),slot.spine());
        expect(target,"Cannot find a path to the shelf");until(lib,Stage::Idle);
        const Vec readingEye=lib.eye();
        expect(lib.open(slot.id),"Failed to select a book");
        until(lib,Stage::Presenting);
        for(int i=0;i<=62 && lib.stage()==Stage::Presenting;++i){
            const auto m=lib.pose().model();
            for(float x:{-.518f,.518f})for(float y:{-coverHeight/2,coverHeight/2})
                for(float z:{-closedThickness/2,closedThickness/2})
                    expect(dot(m.point({x,y,z})-slot.at,slot.outward())>
                           (slot.cabinet<0?Shelf::front:.53f)-slotZ,"Rotation crosses the frame");
            lib.update(tick);
        }
        until(lib,Stage::Reading);
        expect(lib.selected()==slot.id && lib.pose().open==1,"Reader did not finish opening");
        expect(!lib.wingTo(1-slot.wing) && !lib.turn(-1),"Invalid control on the first page");
        for(int i=1;i<spreadCount(slot.id);++i){
            expect(lib.turn(1) && !lib.turn(1),"Simultaneous page turns");
            advance(lib,turnDuration+.03f);expect(lib.spread()==i,"Failed to turn page forward");
        }
        expect(!lib.turn(1),"Page index exceeds the final page");
        for(int i=spreadCount(slot.id)-2;i>=0;--i){
            expect(lib.turn(-1),"Failed to initiate reverse page turn");
            advance(lib,turnDuration+.03f);expect(lib.spread()==i,"Failed to turn page backward");
        }
        const float baseSize=lib.pose().scale.x;
        lib.orbit(82,-47);lib.zoom(4);advance(lib,.35f);
        expect(lib.pose().yaw>.35f && lib.pose().scale.x>baseSize*1.34f,"Orbit or zoom has no effect");
        lib.turn(1);advance(lib,.22f);lib.close();until(lib,Stage::Idle);
        expect(lib.selected()==-1,"Book remains selected after closing");
        expect(dot(lib.eye()-readingEye,lib.eye()-readingEye)<.0001f,"Closing the book moved the reader");
        auto hit=lib.project(slot.spine(),proofSize);
        expect(lib.pick(hit,proofSize)==slot.id,"Returned spine lost its selection");
    }
    for(int wing:{0,1}){
        Library lib;lib.wingTo(wing);until(lib,Stage::Idle);
        for(auto size:{proofSize,Size{720,490},Size{1920,1080}}){
            for(const auto& slot:positions())if(bookSection(slot.id)==Section::Children && slot.cabinet<0 && slot.wing==wing){
                const auto p=lib.project(slot.spine(),size);
                expect(lib.pick(p,size)==slot.id,"Camera selected a different spine");
            }
            for(float y:Shelf::levels){
                const auto p=lib.project({wingX(wing),y-.041f,.66f},size);
                expect(lib.pick(p,size)==-1,"Board unexpectedly intercepts book selection");
            }
        }
    }
    for(float t:{.13f,.58f,1.14f}){
        Library lib;lib.open(20);advance(lib,t);lib.close();until(lib,Stage::Idle);
        expect(lib.open(0),"Cannot reopen after cancelling");
        until(lib,Stage::Reading);expect(lib.selected()==0,"Previous book remains active");
    }
    const auto open=panels(1);
    expect(std::abs(open.front.point({1,0,0}).x+1)<.0001f &&
           std::abs(open.rear.point({1,0,0}).x-1)<.0001f,"Open book covers are on the same side");
    for(int i=0;i<=240;++i){
        const auto hinge=panels(i/240.f);
        const float rear=hinge.rear.point({0,0,pageBlock}).z;
        for(int j=0;j<=32;++j){
            const float r=j/32.f;
            for(float d:{-pageBlock,0.f,board}){
                const auto p=hinge.front.point({r,0,d});
                if(p.x>hinge.xOffset+pageInner)expect(p.z>=rear-.0001f,"Cover intersects the page block");
            }
            const auto p=leafPoint(r*(pageOuter-pageInner),0,i/240.f);
            expect(p.z>pageBlock,"Turning leaf intersects paper");
        }
    }
    std::cout<<"OK: two rooms and a doorway, "<<bookCount<<" volumes, 241 hinge positions, "
             <<popupCount<<" pop-up scenes, camera, collisions, paths and shelf return.\n";
}
namespace {
void saveFrame(Size s,const std::filesystem::path& path){
    std::vector<unsigned char> data(size_t(s.w)*s.h*4);
    glReadPixels(0,0,s.w,s.h,gl::bgra,gl::byte,data.data());
    const size_t stride=size_t(s.w)*4;
    for(int i=0;i<s.h/2;++i)
        std::swap_ranges(data.begin()+i*stride,data.begin()+(i+1)*stride,data.begin()+(s.h-1-i)*stride);
    #ifdef _WIN32
    auto* image=cairo_image_surface_create_for_data(data.data(),CAIRO_FORMAT_RGB24,s.w,s.h,int(stride));
#else
    auto* image=cairo_image_surface_create_for_data(data.data(),1,s.w,s.h,int(stride));
#endif
    #ifdef _WIN32
    const auto utf8=path.u8string();
    const int result=cairo_surface_write_to_png(image,reinterpret_cast<const char*>(utf8.c_str()));
#else
    const int result=cairo_surface_write_to_png(image,path.c_str());
#endif
    cairo_surface_destroy(image);
    expect(result==0,"Failed to save render frame");
}
void send(Interface& ui,SDL_Event e){
    expect(SDL_PushEvent(&e)==1,"Failed to push SDL event");
    SDL_Event event{};
    while(SDL_PollEvent(&event))ui.event(event,nullptr);
}
void key(Interface& ui,int k){SDL_Event e{};e.type=sdl::keyDown;e.key.keysym.sym=k;send(ui,e);}
void type(Interface& ui,const char* t){
    SDL_Event input{};
    input.type=sdl::text;
    std::strncpy(input.text.text,t,sizeof(input.text.text)-1);
    ui.event(input,nullptr);
}
void move(Interface& ui,Point p){SDL_Event e{};e.type=sdl::motion;e.motion.x=int(p.x);e.motion.y=int(p.y);send(ui,e);}
void mouse(Interface& ui,Point p,bool pressed){
    SDL_Event e{};e.type=pressed?sdl::down:sdl::up;e.button.button=1;e.button.x=int(p.x);e.button.y=int(p.y);send(ui,e);
}
} // namespace

int renderTest(const std::filesystem::path& dir,bool desktopMode){
    #ifdef _WIN32
    if(!desktopMode)_putenv_s("SDL_VIDEODRIVER","offscreen");
#else
    if(!desktopMode)setenv("SDL_VIDEODRIVER","offscreen",1);
#endif
    std::filesystem::create_directories(dir);
    Desktop desktop(proofSize,!desktopMode);
    Renderer renderer;
    Interface ui(proofSize);
    float seconds=0;
    std::cout<<"OpenGL: "<<glGetString(gl::renderer)<<'\n';
    auto draw=[&]{
        renderer.draw(ui.lib,proofSize,seconds);
        Paint hud(proofSize);ui.paint(hud);renderer.hud(&hud);
    };
    auto snap=[&](std::string_view name){
        draw();
        saveFrame(proofSize,dir/(std::string(name)+".png"));
        const auto s=renderer.stats();
        expect(glGetError()==0,"OpenGL error during test");
        std::cout<<name<<": "<<s.calls<<" llamadas, "<<s.triangles<<" triángulos\n";
    };
    auto snapCompact=[&](std::string_view name){
        const Size compact{720,490};
        ui.resize(compact);
        renderer.draw(ui.lib,compact,seconds);
        Paint hud(compact);ui.paint(hud);renderer.hud(&hud);
        saveFrame(compact,dir/(std::string(name)+".png"));
        expect(glGetError()==0,"OpenGL error at minimum window size");
        ui.resize(proofSize);
    };
    snap("01-entrada");
    const auto firePixel=ui.lib.project(hearthAt+Vec{0,.88f,0},proofSize);
    auto sampleFlames=[&]{
        constexpr int side=108;
        std::array<unsigned char,side*side*4> samples{};
        glReadPixels(int(firePixel.x)-side/2,proofSize.h-int(firePixel.y)-side/2,
                     side,side,gl::bgra,gl::byte,samples.data());
        return samples;
    };
    const auto firstFlames=sampleFlames();
    seconds=.48f;draw();
    const auto laterFlames=sampleFlames();
    int changed=0;
    for(size_t i=0;i<firstFlames.size();i+=4)
        changed+=std::abs(firstFlames[i+2]-laterFlames[i+2])>35;
    expect(changed>125,"Hearth remains static");
    Library hearthView;
    expect(hearthView.travel({3.74f,walkingEye,10.2f},hearthAt+Vec{0,1.f,0}),
           "Camera cannot reach the side of the hearth");
    until(hearthView,Stage::Idle);
    renderer.draw(hearthView,proofSize,seconds);
    saveFrame(proofSize,dir/"01a-fogata-lateral.png");
    for(float side:{-1.f,1.f}){
        Library perimeterView;
        expect(perimeterView.travel({side*4.16f,walkingEye,4.40f},{side*13.75f,.75f,11.85f}),
               "No route around the room perimeter");
        until(perimeterView,Stage::Idle);
        renderer.draw(perimeterView,proofSize,seconds);
        saveFrame(proofSize,dir/(side<0?"01a-estantes-izquierdos.png":"01a-estantes-derechos.png"));
    }
    Library rearView;
    expect(rearView.travel({0,walkingEye,4.92f},{0,.08f,roomFront-caseDepth}),
           "No route to the rear shelves");
    until(rearView,Stage::Idle);
    renderer.draw(rearView,proofSize,seconds);
    saveFrame(proofSize,dir/"01a-estantes-posteriores.png");
    auto lookWithMouse=[&](int dx,int dy){
        SDL_Event mouse{};mouse.type=sdl::down;mouse.button.button=3;
        mouse.button.x=640;mouse.button.y=400;send(ui,mouse);
        mouse={};mouse.type=sdl::motion;mouse.motion.x=640;mouse.motion.y=400;
        mouse.motion.xrel=dx;mouse.motion.yrel=dy;send(ui,mouse);
        mouse={};mouse.type=sdl::up;mouse.button.button=3;send(ui,mouse);
    };
    const Vec initialView=ui.lib.forward();
    lookWithMouse(695,8);
    expect(dot(initialView,ui.lib.forward())<-.8f,"Mouse cannot look toward the rear wall");
    snap("01b-pared-posterior");
    lookWithMouse(-455,-47);snap("01c-lateral-y-techo");
    const Vec startWalk=ui.lib.eye();
    key(ui,'w');expect(ui.walking(),"W does not activate continuous movement");advance(ui,.72f);
    SDL_Event released{};released.type=sdl::keyUp;released.key.keysym.sym='w';send(ui,released);
    advance(ui,.29f);
    expect(!ui.walking() && walkable(ui.lib.eye()) && dot(ui.lib.eye()-startWalk,ui.lib.eye()-startWalk)>.50f,
           "W does not move the camera in the room");
    const Vec beforeWheel=ui.lib.eye();
    SDL_Event wheel{};wheel.type=sdl::wheel;wheel.wheel.y=1;wheel.wheel.preciseY=1;send(ui,wheel);
    advance(ui,.40f);
    expect(walkable(ui.lib.eye()) && dot(ui.lib.eye()-beforeWheel,ui.lib.eye()-beforeWheel)>.25f,
           "Mouse wheel does not allow moving without a keyboard");
    snap("01d-avance-en-pasillo");
    key(ui,9);
    const Vec beforeTab=ui.lib.forward();
    SDL_Event relative{};relative.type=sdl::motion;relative.motion.xrel=40;relative.motion.yrel=99;
    send(ui,relative);
    expect(dot(beforeTab,ui.lib.forward())<.92f,"Tab did not enable relative mouse mode");
    snap("01e-mesas-de-lectura");
    key(ui,27);
    const Vec afterEscape=ui.lib.forward();
    send(ui,relative);
    expect(dot(afterEscape,ui.lib.forward())>.999f,"Esc did not release mouse capture");
    expect(ui.lib.wingTo(0),"Cannot return to the left wing");until(ui,Stage::Idle);
    constexpr int mars=17;
    expect(popupIndex(mars,0)==int(Figure::Mars),"Mars book did not open with terrain and rover");
    const auto at=ui.lib.project(positions()[mars].at+Vec{0,0,shelfDepth/2},proofSize);
    move(ui,at);advance(ui,.4f);expect(ui.lib.hovered()==mars,"Pointer event failed to detect the spine");
    snap("02-estante-superior");
    const Vec from=ui.lib.eye();
    mouse(ui,at,true);mouse(ui,at,false);
    expect(ui.lib.stage()==Stage::Traveling && ui.lib.selected()==-1,"Distant click opened book without approach");
    snap("02a-acercamiento");
    until(ui,Stage::Extracting);
    expect(ui.lib.selected()==mars && dot(ui.lib.eye()-from,ui.lib.eye()-from)>3.f,
           "Click did not move the reader to the selected book");
    advance(ui,.28f);snap("03-extraccion-superior");
    advance(ui,.54f);snap("04-portada");
    advance(ui,.32f);snap("05-bisagra");
    until(ui,Stage::Reading);snap("06-abierto");snapCompact("06c-libro-ventana-minima");
    key(ui,sdl::f2);snap("06b-vista-3d");key(ui,sdl::f2);
    const float pausedAt=ui.sceneTime(.56f);
    const Point pauseButton{116,42};
    mouse(ui,pauseButton,true);mouse(ui,pauseButton,false);
    expect(!ui.motionEnabled() && ui.sceneTime(1.f)==pausedAt,"Button did not pause animation");
    snap("06f-pausa");
    key(ui,'m');
    expect(ui.motionEnabled() && ui.sceneTime(.38f)>pausedAt,"M did not resume animation");
    for(const Size size:{Size{720,490},proofSize,Size{1920,1080}}){
        const auto openBook=ui.lib.pose().model();
        const Point nearLeft=ui.lib.project(openBook.point({-1,-coverHeight/2,0}),size);
        const Point nearRight=ui.lib.project(openBook.point({1,-coverHeight/2,0}),size);
        const Point farLeft=ui.lib.project(openBook.point({-1,coverHeight/2,0}),size);
        const Point farRight=ui.lib.project(openBook.point({1,coverHeight/2,0}),size);
        expect(nearLeft.x>0 && nearRight.x<size.w && nearLeft.y<size.h &&
               std::abs(nearLeft.x+nearRight.x-size.w)<size.w*.02f &&
               nearRight.x-nearLeft.x>(farRight.x-farLeft.x)*1.45f &&
               nearLeft.y-farLeft.y>size.h*.48f,
               "Open-reader frame lost its pose or cropped the page edge");
    }
    const std::array<float,2> phases{.57f,2.28f};
    using Sample=std::array<unsigned char,340*190*4>;
    auto previousFrame=std::make_unique<Sample>();
    auto nextFrame=std::make_unique<Sample>();
    for(size_t i=0;i<phases.size();++i){
        seconds=phases[i];
        snap(i==0?"06d-marte-movimiento-A":"06e-marte-movimiento-B");
        glReadPixels(260,450,340,190,gl::bgra,gl::byte,i==0?previousFrame->data():nextFrame->data());
    }
    int movedPixels=0;
    for(size_t i=0;i<previousFrame->size();i+=4)
        movedPixels+=std::abs(int((*previousFrame)[i+2])-int((*nextFrame)[i+2]))>20;
    expect(movedPixels>80,"Mars scene remains static");
    auto movingSnapshot=ui.lib;
    expect(movingSnapshot.turn(1),"Cannot verify animation completion");
    advance(movingSnapshot,.57f);
    renderer.draw(movingSnapshot,proofSize,.6f);
    glReadPixels(260,450,340,190,gl::bgra,gl::byte,previousFrame->data());
    renderer.draw(movingSnapshot,proofSize,2.3f);
    glReadPixels(260,450,340,190,gl::bgra,gl::byte,nextFrame->data());
    for(size_t i=0;i<previousFrame->size();i+=4)
        expect(std::abs(int((*previousFrame)[i+2])-int((*nextFrame)[i+2]))<=3,
               "Figure floats apart during the page turn");
    const Point disabledPrevious{568,752};
    mouse(ui,disabledPrevious,true);mouse(ui,disabledPrevious,false);
    expect(ui.lib.stage()==Stage::Reading && ui.lib.spread()==0,
           "Disabled arrow closed the book");
    key(ui,sdl::right);advance(ui,.20f);snap("07a-plegado");
    advance(ui,.37f);snap("07b-hoja");
    advance(ui,.37f);snap("07c-pliegue-nuevo");
    advance(ui,.35f);expect(ui.lib.spread()==1,"Right arrow did not advance");snap("08-paginas-3-4");
    key(ui,sdl::left);advance(ui,.62f);snap("09-regresar-hoja");advance(ui,.66f);
    expect(ui.lib.spread()==0,"Left arrow did not go back");
    const float baseSize=ui.lib.pose().scale.x;
    const auto center=ui.lib.project(ui.lib.pose().model().point({.46f,-.22f,pageBlock}),proofSize);
    const Point off{center.x+70,center.y-40};
    mouse(ui,center,true);move(ui,off);mouse(ui,off,false);
    wheel.wheel.y=2;wheel.wheel.preciseY=2;send(ui,wheel);
    advance(ui,.51f);
    expect(ui.lib.pose().yaw>.3f && ui.lib.pose().scale.x>baseSize*1.15f,"Dragging or scrolling had no effect");
    snap("10-orbita");
    key(ui,27);advance(ui,.40f);snap("11a-cerrar-figura");
    until(ui,Stage::Idle);snap("11b-devuelto");
    key(ui,sdl::right);until(ui,Stage::Idle);expect(ui.lib.wing()==1,"Arrow did not change wing");
    snap("12-ala-ii");
    Interface catalogControls(proofSize);
    key(catalogControls,'/');expect(catalogControls.catalogVisible(),"Catalog did not open");
    const int allChildren=catalogControls.resultCount();
    key(catalogControls,9);key(catalogControls,13);
    expect(catalogControls.catalogVisible() && catalogControls.resultCount()>0 &&
           catalogControls.resultCount()<allChildren,
           "Genre filter did not respond to keyboard");
    renderer.draw(catalogControls.lib,proofSize,seconds);
    Paint focusedCatalog(proofSize);catalogControls.paint(focusedCatalog);renderer.hud(&focusedCatalog);
    saveFrame(proofSize,dir/"13a-filtro-con-foco.png");
    key(catalogControls,9);key(catalogControls,13);
    expect(!catalogControls.catalogVisible(),"Cannot close catalog using keyboard focus");
    key(ui,'/');expect(ui.catalogVisible(),"Catalog did not open");
    type(ui,"martian stones");
    expect(ui.resultCount()==1,"Accent-insensitive search returned no result");snap("13-catalogo");
    key(ui,13);until(ui,Stage::Reading);expect(ui.lib.selected()==31,"Catalog opened the wrong book");snap("14-otro-libro");

    Library adultReader;
    expect(adultReader.enter(Section::Adult),"Cannot prepare distant adult book opening test");
    until(adultReader,Stage::Idle);
    constexpr int urbanHabitat=originalCount+1;
    const Vec farEye=adultReader.eye();
    expect(adultReader.open(urbanHabitat) && adultReader.stage()==Stage::Traveling &&
           adultReader.selected()==-1,"Distant adult book opened before camera arrived");
    for(int time=0;time<1600 && adultReader.stage()==Stage::Traveling;++time){
        adultReader.update(tick);
        expect(walkable(adultReader.eye()),"Adult room approach intersects furniture");
    }
    const Vec displacement=adultReader.eye()-farEye;
    expect(dot(displacement,displacement)>4.f && adultReader.selected()==urbanHabitat,
           "Camera did not approach the adult shelf");
    until(adultReader,Stage::Reading);
    expect(adultReader.turn(1),"Cannot turn the first adult spread");
    advance(adultReader,turnDuration+.05f);
    expect(adultReader.turn(1),"Cannot open Water and Priorities");
    advance(adultReader,turnDuration+.05f);
    renderer.draw(adultReader,proofSize,seconds);
    saveFrame(proofSize,dir/"14a-agua-y-prioridades-sin-silla.png");

    std::array<bool,popupCount> seen{};
    auto coverAll=[&]{
        for(int i=0;i<spreads;++i){
            const int model=popupIndex(ui.lib.selected(),i);
            if(!seen[model]){
                snap("15-modelo-"+std::to_string(model));
                seen[model]=true;
            }
            if(i+1<spreads){
                key(ui,sdl::right);advance(ui,turnDuration+.035f);
                expect(ui.lib.spread()==i+1,"Figure catalog did not advance");
            }
        }
    };
    coverAll();
    for(int book:{0,1,2,3,4,5,6}){
        ui.lib.close();until(ui,Stage::Idle);
        expect(ui.lib.open(book),"Destination book did not open");
        until(ui,Stage::Reading);
        if(book==0){
            snap("15a-tierra-frente");
            ui.lib.orbit(100,-36);advance(ui,.48f);snap("15b-tierra-lateral");
            ui.lib.orbit(-100,36);advance(ui,.48f);
        }
        coverAll();
    }
    key(ui,sdl::left);
    Paint staticHud(proofSize);ui.paint(staticHud);
    renderer.hud(&staticHud);
    const auto start=Clock::now();
    for(int i=0;i<48;++i){
        ui.step(1.f/60);
        renderer.draw(ui.lib,proofSize);renderer.hud(nullptr);glFinish();
    }
    std::cout<<"48-frame render at 1280×800: "
        <<std::chrono::duration<double,std::milli>(Clock::now()-start).count()/48
        <<" ms/cuadro promedio.\n";
    for(int i=0;i<30;++i){
        ui.lib.close();until(ui,Stage::Idle);renderer.draw(ui.lib,proofSize);
        ui.lib.open(28+i%2);until(ui,Stage::Reading);renderer.draw(ui.lib,proofSize);
    }
    expect(glGetError()==0,"OpenGL error after 30 cycles");
    std::cout<<"30 ciclos abrir/cerrar sin errores OpenGL.\n";
    for(int id=firstImport;id<firstImport+importedChildren;++id){
        ui.lib.close();until(ui,Stage::Idle);
        expect(ui.lib.visit(id),"Cannot reach an imported story");
        until(ui,Stage::Reading);
        for(int spread=0;spread<spreadCount(id);++spread){
            seen[popupIndex(id,spread)]=true;
            snap("16-infantil-"+std::to_string(id)+"-"+std::to_string(spread));
            if(spread+1<spreadCount(id)){
                expect(ui.lib.turn(1),"Illustrated page does not turn");
                advance(ui,turnDuration+.04f);
            }
        }
    }
    ui.lib.close();until(ui,Stage::Idle);
    const Point roomButton{119,43};
    mouse(ui,roomButton,true);mouse(ui,roomButton,false);
    until(ui,Stage::Idle);
    expect(ui.lib.section()==Section::Adult && ui.resultCount()==adultCount,"Room button did not synchronize the adult room");
    snap("17-sala-adulta");
    Library doorwayView;
    expect(doorwayView.travel({roomHalfWidth-4.7f,walkingEye,doorZ},{roomHalfWidth,.58f,doorZ}),
           "Cannot reach the adult room sign");
    until(doorwayView,Stage::Idle);
    renderer.draw(doorwayView,proofSize,seconds);
    saveFrame(proofSize,dir/"17a-acceso.png");
    Interface lecternView(proofSize);
    const auto restingBook=onLectern();
    expect(restingBook.open==1,"Lectern book starts closed");
    const Vec plaque=restingBook.at;
    for(float side:{-1.f,1.f}){
        const Vec approach{roomHalfWidth+side*1.1f,walkingEye,doorZ-.15f};
        expect(lecternView.lib.travel(approach,plaque),"Lectern blocks access to the door");
        until(lecternView,Stage::Idle);
        const auto target=lecternView.lib.project(plaque,proofSize);
        expect(lecternView.lib.pickReferences(target,proofSize),"Lectern cannot be reached from the corridor");
        mouse(lecternView,target,true);mouse(lecternView,target,false);
        expect(lecternView.lib.stage()==Stage::Traveling,"Lectern reader starts without approach");
        until(lecternView,Stage::Reading);
        expect(lecternView.lib.selected()==referenceBookId && lecternView.lib.spread()==0 &&
               lecternView.lib.pose().model().a==restingBook.model().a,
               "Approaching the camera moved the lectern book");
        key(lecternView,27);
        until(lecternView,Stage::Idle);
        expect(lecternView.lib.selected()<0,"Reference book did not return to the lectern");
    }
    renderer.draw(lecternView.lib,proofSize,seconds);
    saveFrame(proofSize,dir/"17b-atril.png");
    const auto sourceSnap=[&](const char* name){
        renderer.draw(lecternView.lib,proofSize,seconds);
        Paint hud(proofSize);lecternView.paint(hud);renderer.hud(&hud);
        saveFrame(proofSize,dir/name);
    };
    const auto startEye=lecternView.lib.eye();
    key(lecternView,'f');
    advance(lecternView,.052f);
    expect(lecternView.lib.stage()==Stage::Traveling && lecternView.lib.selected()<0 &&
           dot(lecternView.lib.eye()-startEye,lecternView.lib.eye()-startEye)>.01f,
           "Lectern shortcut does not approach the open book");
    sourceSnap("17c-acercamiento-atril.png");
    until(lecternView,Stage::Reading);
    sourceSnap("17d-citas-marte.png");
    const auto referenceBook=lecternView.lib.pose().model();
    for(Size size:{Size{720,490},proofSize,Size{1920,1080}})
        for(float turn:{.25f,.40f,.50f,.60f,.75f})
            for(float side:{-.5f,.5f}){
                const auto corner=leafPoint(pageOuter-pageInner,side*pageHeight,turn);
                const auto screen=lecternView.lib.project(referenceBook.point(corner),size);
                expect(screen.x>0 && screen.x<size.w && screen.y>0 && screen.y<size.h,
                       "Lectern page extends outside the screen");
            }
    for(int side:{-1,1}){
        const Vec link{side*.54f,(.5f-referenceLinkY(0))*pageHeight,pageBlock};
        const auto linkPoint=lecternView.lib.project(referenceBook.point(link),proofSize);
        const auto page=lecternView.lib.referencePage(linkPoint,proofSize);
        expect(page && int(page->x)==(side+1)/2 && std::abs(page->y-referenceLinkY(0))<.002f,
               "Reference link lost alignment with the 3D book");
    }
    key(lecternView,sdl::right);
    advance(lecternView,turnDuration*.5f);
    expect(lecternView.lib.target()==1 && lecternView.lib.turning()<1,
           "References changed without turning a page");
    sourceSnap("17e-hoja-del-atril.png");
    advance(lecternView,turnDuration*.5f+.05f);
    expect(lecternView.lib.spread()==1 && !lecternView.lib.turn(1),"Reference book has no bounds");
    sourceSnap("17f-citas-luna.png");
    key(lecternView,27);until(lecternView,Stage::Idle);
    expect(lecternView.lib.lecternSpread()==1,"Lectern book lost the lunar pages");
    key(lecternView,'f');until(lecternView,Stage::Reading);
    expect(lecternView.lib.spread()==1,"Revisiting the lectern changed its pages");
    key(lecternView,sdl::left);
    advance(lecternView,turnDuration+.05f);
    expect(lecternView.lib.spread()==0,"Cannot return from Moon references to Mars references");
    key(lecternView,27);until(lecternView,Stage::Idle);
    key(ui,'k');expect(ui.catalogVisible() && ui.resultCount()==adultCount,"Adult catalog mixed room collections");
    type(ui,"dawn of particles");expect(ui.resultCount()==1,"Technical reading was not moved");
    key(ui,27);
    expect(ui.lib.travel({roomHalfWidth+2.3f,walkingEye,doorZ},{roomHalfWidth-3.1f,walkingEye,doorZ}),
           "Cannot reach the doorway");
    until(ui,Stage::Idle);snap("18-puerta");
    key(ui,'w');advance(ui,1.28f);send(ui,released);advance(ui,.31f);
    expect(ui.lib.section()==Section::Children && ui.resultCount()==childrenCount,
           "Walking between rooms did not synchronize the children's catalog");
    key(ui,'g');until(ui,Stage::Idle);
    key(ui,'k');expect(ui.resultCount()==adultCount,"Room return kept the previous room's filter");
    snap("19-catalogo-adulto");snapCompact("19a-catalogo-ventana-minima");
    type(ui,"signal shift");expect(ui.resultCount()==1,"Cannot find a title in the adult room");
    key(ui,13);until(ui,Stage::Reading);
    expect(ui.lib.selected()==originalCount,"Adult catalog opened the wrong book");
    for(int id=0;id<bookCount;++id){
        if(bookSection(id)!=Section::Adult)continue;
        if(id!=ui.lib.selected()){
            ui.lib.close();until(ui,Stage::Idle);
            expect(ui.lib.visit(id),"Cannot approach the adult book");
            until(ui,Stage::Reading);
        }
        for(int i=0;i<adultSpreads;++i){
            seen[popupIndex(id,i)]=true;
            snap("20-lectura-"+std::to_string(id)+"-"+std::to_string(i+1));
            if(i+1==adultSpreads)break;
            expect(ui.lib.turn(1),"Cannot advance in an adult book");
            if(i==0 && id==originalCount){advance(ui,.60f);snap("20a-vuelta-lectura");}
            advance(ui,turnDuration+.03f);
        }
        expect(!ui.lib.turn(1),"Adult reading exceeds its last page");
    }
    ui.lib.close();until(ui,Stage::Idle);
    mouse(ui,roomButton,true);mouse(ui,roomButton,false);until(ui,Stage::Idle);
    expect(ui.lib.section()==Section::Children && ui.resultCount()==childrenCount,
           "Room button did not return to children's catalog");
    expect(std::count(seen.begin(),seen.end(),true)==popupCount,"A figure was not rendered");
    expect(ui.lib.visit(firstImport+4),"Cannot open Pioneer to verify launch");
    until(ui,Stage::Reading);
    snap("21-pioneer-despegue-inicio");
    glReadPixels(650,390,340,190,gl::bgra,gl::byte,previousFrame->data());
    advance(ui,launchDuration/2);
    snap("22-pioneer-despegue-mitad");
    advance(ui,launchDuration/2+tick);
    snap("23-pioneer-despegue-final");
    glReadPixels(650,390,340,190,gl::bgra,gl::byte,nextFrame->data());
    int launchPixels=0;
    for(size_t i=0;i<previousFrame->size();i+=4)
        launchPixels+=std::abs(int((*previousFrame)[i+2])-int((*nextFrame)[i+2]))>20;
    expect(launchPixels>500,"Rocket does not visibly move in the render");
    // Hold the room's clock still so these differences only measure exhaust.
    std::swap(previousFrame,nextFrame);
    advance(ui,trailFadeDuration/2);
    snap("24-pioneer-estela-disipandose");
    advance(ui,trailFadeDuration/2+tick);
    snap("25-pioneer-estela-disipada");
    glReadPixels(650,390,340,190,gl::bgra,gl::byte,nextFrame->data());
    int trailPixels=0;
    for(size_t i=0;i<previousFrame->size();i+=4)
        trailPixels+=std::abs(int((*previousFrame)[i+2])-int((*nextFrame)[i+2]))>12;
    expect(trailPixels>100,"No visible trail remains after the rocket exits");
    advance(ui,.5f);
    snap("26-pioneer-sin-reaparicion");
    glReadPixels(650,390,340,190,gl::bgra,gl::byte,previousFrame->data());
    expect(*previousFrame==*nextFrame,"Trail or rocket reappears after launch");
    return 0;
}
} // namespace biblioteca

