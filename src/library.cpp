#include "library.hpp"
#include "locale.hpp"
#include "adult_books.hpp"
#include "child_text.hpp"
#include "imported_books.hpp"
#include "technical_notes.hpp"
#include "stories.hpp"
#include "room_layout.hpp"
#include <cstdio>
#include <limits>

namespace biblioteca {
namespace {
#include "catalog.inc"
#include "catalog_es.inc"
float slope(Size s) {
    const float aspect=float(s.w)/std::max(1,s.h);
    return std::tan(std::min(64.f,48.f*std::max(1.f,1.55f/aspect))*pi/360);
}
float approach(float v,float goal,float dt,float speed) {
    return std::abs(goal-v)<.001f ? goal : v+(goal-v)*(1-std::exp(-dt*speed));
}
} // namespace

Mat Mat::move(Vec v) { Mat m; m.a[12]=v.x; m.a[13]=v.y; m.a[14]=v.z; return m; }
Mat Mat::scale(Vec v) { Mat m; m.a[0]=v.x; m.a[5]=v.y; m.a[10]=v.z; return m; }
Mat Mat::rx(float t) { Mat m;const float c=std::cos(t),s=std::sin(t);m.a[5]=m.a[10]=c;m.a[9]=-s;m.a[6]=s;return m; }
Mat Mat::ry(float t) { Mat m;const float c=std::cos(t),s=std::sin(t);m.a[0]=m.a[10]=c;m.a[2]=-s;m.a[8]=s;return m; }
Mat Mat::operator*(const Mat& b) const {
    Mat out;
    for (int col=0;col<4;++col) for(int row=0;row<4;++row) {
        float value=0;
        for(int k=0;k<4;++k) value+=a[k*4+row]*b.a[col*4+k];
        out.a[col*4+row]=value;
    }
    return out;
}
Vec Mat::point(Vec v,bool direction) const {
    const Vec p={a[0]*v.x+a[4]*v.y+a[8]*v.z,a[1]*v.x+a[5]*v.y+a[9]*v.z,a[2]*v.x+a[6]*v.y+a[10]*v.z};
    if(direction) return p;
    return (p+Vec{a[12],a[13],a[14]})/(a[3]*v.x+a[7]*v.y+a[11]*v.z+a[15]);
}
Bounds Bounds::box(Vec c,Vec s) {return {c-s/2,c+s/2};}
bool Bounds::overlaps(const Bounds& b) const {
    for(int i=0;i<3;++i) if(std::min(max[i],b.max[i])-std::max(min[i],b.min[i])<=.0001f) return false;
    return true;
}
std::optional<float> Bounds::hit(Ray r) const {
    float near=0,far=std::numeric_limits<float>::infinity();
    for(int i=0;i<3;++i) {
        if(std::abs(r.direction[i])<.00001f) {
            if(r.origin[i]<min[i] || r.origin[i]>max[i]) return {};
            continue;
        }
        float a=(min[i]-r.origin[i])/r.direction[i], b=(max[i]-r.origin[i])/r.direction[i];
        if(a>b)std::swap(a,b);
        near=std::max(near,a);far=std::min(far,b);
        if(near>far)return {};
    }
    return near;
}
std::string Book::number(int id) const {
    std::string num=std::to_string(id+1); return std::string(3-num.size(),'0')+num;
}
const std::array<Book,bookCount>& books() {
    static std::array<Book,bookCount> catalog{};
    static int lastLanguage=-1;
    const int currentLanguage=isEnglish()?1:0;
    if(lastLanguage!=currentLanguage){
    const auto result=[] {
        std::array<Book,bookCount> result{};
        for(int i=0;i<originalCount;++i){
            const auto& b=isEnglish()?seeds[i]:seedsEs[i];const auto& c=palette[(i%7)*3+(i/7)%3];
            result[i]={b[0],b[1],b[2],c[0],c[1],.245f+((i*17+11)%24)*.005f,
                       1.13f+((i*29+7)%27)*.004f,1998+((i*11+13)%28)};
        }
        for(int i=0;i<originalAdultCount;++i){
            const auto& b=adultBooks()[i];
            result[originalCount+i]={b.title,b.genre,b.description,b.binding,"#eddebd",
                                     .29f+.019f*i,1.19f+.020f*(i%3),2026};
        }
        for(int i=0;i<importedChildren+importedAdults;++i){
            const auto& b=importedBooks()[i].text;
            result[firstImport+i]={b.title,b.genre,b.description,b.binding,"#eddebd",
                                   .265f+.021f*(i%5),1.14f+.028f*(i%4),2026};
        }
        return result;
    }();
    catalog=result;
    lastLanguage=currentLanguage;
    }
    return catalog;
}
std::string pageText(int id,int page) {
    if(id>=originalCount){
        const auto& volume=id>=firstImport?importedBooks()[id-firstImport].text:adultBooks()[id-originalCount];
        const auto& passage=volume.chapters[page/2];
        return std::string(page%2?passage.right:passage.left);
    }
    if(bookSection(id)==Section::Adult){
        const auto& incident=chapter(id,page);
        return std::string(incident.left)+" "+std::string(incident.right)+"\n"+
               std::string(technicalNote(destination(id),page));
    }
    const auto& passage=childSpread(id,page/2);
    const auto& source=chapter(id,page/2);
    const auto& base=page%2?passage.right:passage.left;
    // Enriquecer la lectura sin inventar otro cuento: reutiliza la primera
    // oración del registro adulto correspondiente al mismo capítulo/PDF.
    const auto& detail=page%2?source.right:source.left;
    const auto end=detail.find('.');
    auto excerpt=detail.substr(0,end==std::string_view::npos?detail.size():end+1);
    // Keep the added mission note readable beside the pop-up, even in the
    // longest child spread. It is still taken from the same source chapter.
    constexpr std::size_t excerptLimit=58;
    if(excerpt.size()>excerptLimit){
        excerpt=excerpt.substr(0,excerptLimit);
        if(const auto cut=excerpt.rfind(' ');cut>36)excerpt=excerpt.substr(0,cut);
    }
    return std::string(base)+(isEnglish()?"\n\nMission log: ":"\n\nRegistro de misión: ")+std::string(excerpt);
}
float wingX(int wing) { return wing==0?-5.1f:5.1f; }
float ease(float x) { x=std::clamp(x,0.f,1.f);return x*x*x*(10+x*(-15+6*x)); }
const std::array<Slot,bookCount>& positions() {
    static const auto slots=[] {
        std::array<Slot,bookCount> result{};
        constexpr float gap=.042f;
        constexpr std::array childBooks{8,11,12,21,22,23,24,27,32,35,38,40,46,47,48,49,50,51,52,53};
        constexpr std::array childBays{1,2,3,4,5,6,7,8,9,10,11,13,14,16};
        constexpr std::array adultBooks{9,15,18,25,26,36,37,41,54,55,56,57,58};
        constexpr std::array adultBays{17,18,19,20,21,22,23,24};
        std::array<bool,bookCount> outside{};
        const auto place=[&](const auto& ids,const auto& bays){
            for(size_t i=0;i<ids.size();++i){
                const int id=ids[i],bay=bays[i%bays.size()];
                const auto& cabinet=displayCases()[bay];
                const int row=i<bays.size()?2:1;
                const float heading=displayCaseHeading(cabinet.wall);
                const float x=splitCase(cabinet)?cabinet.width*.26f:
                              (i%2==0?-1.f:1.f)*cabinet.width*.15f;
                const Mat transform=Mat::move(cabinet.at)*Mat::ry(heading);
                auto& slot=result[id];
                slot={id,0,row,{},bay,heading};
                slot.at=transform.point({x,displayLevel(row)+slot.height()*.5f,slotZ});
                outside[id]=true;
            }
        };
        place(childBooks,childBays);
        place(adultBooks,adultBays);
        for(const auto section:{Section::Children,Section::Adult}){
            const bool child=section==Section::Children;
            std::vector<int> ids;
            for(int id=0;id<bookCount;++id)if(!outside[id] && bookSection(id)==section)ids.push_back(id);
            const int shelves=child?6:3;
            size_t first=0;
            for(int index=0;index<shelves;++index){
                const int wing=child?index/3:0,row=index%3;
                const size_t count=ids.size()/shelves+(size_t(index)<ids.size()%shelves);
                float width=gap*(count-1);
                for(size_t i=first;i<first+count;++i)width+=books()[ids[i]].width;
                float x=(child?wingX(wing):adultShelfX)-width*.5f;
                for(size_t i=first;i<first+count;++i){
                    const int id=ids[i];
                    const auto& b=books()[id];
                    result[id]={id,wing,row,{x+b.width/2,Shelf::levels[row]+b.height/2,slotZ}};
                    x+=b.width+gap;
                }
                first+=count;
            }
        }
        return result;
    }();
    return slots;
}
float Slot::height() const{return cabinet<0?books()[id].height:.927f+.008f*(id%3);}
Vec Slot::outward() const{return Mat::ry(heading).point({0,0,1},true);}
Vec Slot::spine() const{return at+outward()*(shelfDepth*.5f);}
Vec Slot::approachPoint() const{
    const Vec base{at.x,walkingEye,at.z};
    const auto normal=outward();
    if(cabinet<0)return base+normal*(4.22f-slotZ);
    const Vec along{normal.z,0,-normal.x};
    for(float distance:{3.12f,3.72f,2.66f,4.34f})
        for(float side:{0.f,-.58f,.58f,-1.17f,1.17f}){
            const Vec candidate=base+normal*distance+along*side;
            if(walkable(candidate))return candidate;
        }
    return base;
}
Bounds Slot::bounds(float push) const {
    const auto normal=outward();
    const float thickness=books()[id].width;
    const Vec size=std::abs(normal.x)>.5f?Vec{shelfDepth,height(),thickness}:Vec{thickness,height(),shelfDepth};
    return Bounds::box(at+normal*push,size);
}
std::array<Bounds,7> Shelf::solids() const {
    auto box=[&](Vec p,Vec s){p.x+=x;return Bounds::box(p,s);};
    return {box({0,.04f,-.56f},{8.26f,4.45f,.19f}),
            box({0,levels[0]-.0775f,.04f},{8.14f,.155f,1.16f}),
            box({0,levels[1]-.0775f,.04f},{8.14f,.155f,1.16f}),
            box({0,levels[2]-.0775f,.04f},{8.14f,.155f,1.16f}),
            box({0,top+.105f,.04f},{8.64f,.21f,1.24f}),
            box({-4.11f,.04f,.03f},{.27f,4.46f,1.2f}),
            box({4.11f,.04f,.03f},{.27f,4.46f,1.2f})};
}
Pose onShelf(int id,float push) {
    const auto& b=books()[id];
    const auto& slot=positions()[id];
    return {slot.at+slot.outward()*push,{shelfDepth/(1+spineEdge),slot.height()/coverHeight,b.width/closedThickness},
            slot.heading+pi/2,0,0};
}
Pose onLectern(){
    constexpr float yaw=-pi/2,pitch=-.56f,scale=.36f;
    const Vec at=referenceLecternAt+Vec{-.02f,1.35f,0};
    const Vec normal=(Mat::ry(yaw)*Mat::rx(pitch)).point({0,0,1},true);
    return {at+normal*(.045f+scale*board+.003f),{scale,scale,scale},yaw,pitch,1};
}
Mat Pose::model() const { return Mat::move(at)*Mat::ry(viewYaw)*Mat::rx(viewPitch)*Mat::ry(yaw)*Mat::rx(pitch)*Mat::scale(scale); }
Panels panels(float opening) {
    const float angle=pi*opening;
    // The hinge moves with the front board so the inner face stays above the rear block.
    const float z=(pageBlock+hingeGap)*(1+std::cos(angle));
    const float x=-(1-spineEdge)/2*(1-opening);
    return {Mat::move({x,0,-z/2}),Mat::move({x,0,z/2})*Mat::ry(-angle),z,x};
}
Vec leafPoint(float radial,float y,float t) {
    const float angle=pi*t, bend=.76f*std::sin(angle);
    const float half=bend*radial/(2*(pageOuter-pageInner));
    const float arc=radial*(std::abs(half)<.0001f?1.f:std::sin(half)/half);
    return {pageInner*std::cos(angle)+arc*std::cos(angle-bend*.32f+half),y,
            pageBlock+.0035f+.011f*std::sin(angle)+pageInner*std::sin(angle)+arc*std::sin(angle-bend*.32f+half)};
}
Vec Library::forward() const {
    const float c=std::cos(headPitch_);
    return {std::sin(headYaw_)*c,std::sin(headPitch_),-std::cos(headYaw_)*c};
}
Vec Library::right() const{return {std::cos(headYaw_),0,std::sin(headYaw_)};}
Vec Library::up() const{return cross(right(),forward());}
void Library::look(float dx,float dy){
    if(selected_>=0)return;
    route_.clear();queued_=-1;
    headYaw_=std::remainder(headYaw_+dx*.0042f,2*pi);
    headPitch_=std::clamp(headPitch_-dy*.0042f,-1.34f,1.34f);
}
void Library::walk(float front,float side){
    if(selected_>=0)return;
    route_.clear();queued_=-1;
    eye_=walkStep(eye_,Vec{std::sin(headYaw_),0,-std::cos(headYaw_)}*front+right()*side);
    wing_=eye_.x<0?0:1;
}
Section Library::section() const{return eye_.x<roomHalfWidth?Section::Children:Section::Adult;}
bool Library::enter(Section room){
    if(room==section())return false;
    if(room==Section::Adult)return travel({27.f,walkingEye,10.75f},{adultShelfX,.37f,.5f});
    return travel({11.f,walkingEye,7.70f},{wingX(1),.64f,.5f});
}
bool Library::travel(Vec to,Vec aim,int book){
    if(selected_>=0 || !walkable(to))return false;
    to.y=walkingEye;
    auto path=routeInside(eye_,to);
    if(path.empty())return false;
    route_=std::move(path);waypoint_=0;aim_=aim;queued_=book;hovering_=-1;
    return true;
}
bool Library::wingTo(int w){
    if(section()!=Section::Children || w<0 || w>1)return false;
    return travel({wingX(w),walkingEye,6.65f},{wingX(w),.68f,.5f});
}
bool Library::visit(int id){
    if(id<0 || id>=bookCount || bookSection(id)!=section())return false;
    const auto& slot=positions()[id];
    return travel(slot.approachPoint(),slot.spine(),id);
}
bool Library::open(int id){
    if(selected_>=0 || id<0 || id>=bookCount || bookSection(id)!=section())return false;
    const auto& slot=positions()[id];
    const Vec goal=slot.approachPoint(),offset=eye_-goal;
    const Vec aim=unit(slot.spine()-eye_);
    if(dot(offset,offset)>.012f || dot(aim,forward())<.9997f)
        return visit(id);
    return extract(id);
}
bool Library::openReferences(){
    return travel({referenceLecternAt.x-.90f,walkingEye,referenceLecternAt.z},
                  onLectern().at+Vec{0,.031f,0},referenceBookId);
}
bool Library::extract(int id){
    route_.clear();
    pickedPush_=0;
    if(id!=referenceBookId){
        const auto& slot=positions()[id];
        if(slot.cabinet<0)wing_=slot.wing;
        pickedPush_=offsets_[id];
    }
    queued_=-1;selected_=id;hovering_=-1;
    spread_=to_=id==referenceBookId?lecternSpread_:0;
    flight_=id==referenceBookId?flightEnd:0;
    popupClock_=settle_=0;page_=1;return_=false;
    yaw_=pitch_=goalYaw_=goalPitch_=0;zoom_=goalZoom_=1;
    return true;
}
void Library::close(){
    queued_=-1;route_.clear();
    if(selected_<0 || return_)return;
    if(selected_==referenceBookId){
        if(page_==1)leaveLectern();
        else return_=true;
        return;
    }
    return_=true;goalYaw_=goalPitch_=0;goalZoom_=1;
    settle_=(std::abs(yaw_)+std::abs(pitch_)+std::abs(zoom_-1)>.02f)?.26f:0;
}
void Library::leaveLectern(){
    selected_=-1;return_=false;
    travel({referenceLecternAt.x-1.58f,walkingEye,referenceLecternAt.z+.26f},onLectern().at);
}
bool Library::turn(int sign){
    if(stage()!=Stage::Reading || page_<1 || (sign!=1 && sign!=-1))return false;
    const int next=spread_+sign;
    if(next<0 || next>=spreadCount(selected_))return false;
    to_=next;page_=0;return true;
}
void Library::update(float dt){
    if(!route_.empty()){
        const Vec delta=route_[waypoint_]-eye_;
        const float distance=std::sqrt(dot(delta,delta));
        const float speed=7.55f*dt;
        if(distance<=speed){
            eye_=walkStep(eye_,delta);
            if(waypoint_+1<route_.size())++waypoint_;
        }else eye_=walkStep(eye_,delta*(speed/distance));
        wing_=eye_.x<0?0:1;
        const Vec remaining=route_.back()-eye_;
        const Vec aim=aim_-eye_,ahead=route_[waypoint_]-eye_;
        const float aimYaw=std::atan2(aim.x,-aim.z),routeYaw=std::atan2(ahead.x,-ahead.z);
        const float nearing=ease((4.25f-std::sqrt(dot(remaining,remaining)))/3.90f);
        const float goalYaw=routeYaw+nearing*std::remainder(aimYaw-routeYaw,2*pi);
        const float yawDelta=std::remainder(goalYaw-headYaw_,2*pi);
        const float pitchGoal=nearing*std::atan2(aim.y,std::hypot(aim.x,aim.z));
        headYaw_=std::remainder(headYaw_+approach(0,yawDelta,dt,7.5f),2*pi);
        headPitch_=approach(headPitch_,pitchGoal,dt,7.5f);
        if(dot(remaining,remaining)<=.0001f && std::abs(yawDelta)<.003f && std::abs(headPitch_-pitchGoal)<.003f){
            route_.clear();
            if(queued_>=0)extract(queued_);
        }
    }
    for(int i=0;i<bookCount;++i)offsets_[i]=approach(offsets_[i],i==hovering_?.09f:0,dt,15);
    if(selected_<0)return;
    const bool wasTurning=page_<1;
    if(wasTurning){
        page_=std::min(1.f,page_+dt/turnDuration);
        if(page_==1){spread_=to_;popupClock_=0;}
    }
    if(selected_==referenceBookId){
        lecternSpread_=spread_;
        if(return_ && page_==1)leaveLectern();
        return;
    }
    yaw_=approach(yaw_,goalYaw_,dt,18);pitch_=approach(pitch_,goalPitch_,dt,18);
    zoom_=approach(zoom_,goalZoom_,dt,16);
    if(return_){
        if(page_<1)return;
        if(settle_>0){settle_-=dt;return;}
        flight_=std::max(0.f,flight_-dt);
        if(flight_==0){selected_=-1;return_=false;}
    } else {
        const float previous=flight_;
        flight_=std::min(flightEnd,flight_+dt);
        if(previous==flightEnd && !wasTurning && motionEnabled_)
            popupClock_=std::min(launchSequenceDuration,popupClock_+dt);
    }
}
void Library::hover(int id){hovering_=selected_<0 && queued_<0 ? id : -1;}
void Library::orbit(float dx,float dy){
    if(stage()!=Stage::Reading || selected_==referenceBookId)return;
    goalYaw_=std::clamp(goalYaw_+dx*.007f,-1.23f,1.23f);
    goalPitch_=std::clamp(goalPitch_+dy*.007f,-.43f,.70f);
}
void Library::zoom(float steps){
    if(stage()==Stage::Reading && selected_!=referenceBookId)goalZoom_=std::clamp(goalZoom_*std::exp(.095f*steps),.72f,1.53f);
}
Stage Library::stage() const {
    if(selected_<0)return route_.empty()?Stage::Idle:Stage::Traveling;
    if(return_)return flight_>openStart?Stage::Closing:Stage::Returning;
    if(flight_<extractTime)return Stage::Extracting;
    if(flight_<openStart)return Stage::Presenting;
    return flight_<flightEnd?Stage::Opening:Stage::Reading;
}
bool Library::animating() const {
    if(stage()!=Stage::Idle)return true;
    if(yaw_!=goalYaw_ || pitch_!=goalPitch_ || zoom_!=goalZoom_)return true;
    for(int i=0;i<bookCount;++i)if(offsets_[i]!=(i==hovering_?.09f:0.f))return true;
    return false;
}
Pose Library::pose() const {
    if(selected_==referenceBookId)return onLectern();
    auto p=onShelf(selected_);
    const auto normal=positions()[selected_].outward();
    const auto extracted=p.at+normal*(extractedZ-slotZ);
    const auto origin=p.at;
    if(flight_<=extractTime){
        const float t=ease(flight_/extractTime);
        p.at=origin*(1.f-t)+extracted*t+normal*(pickedPush_*(1.f-t)*(return_?t:1.f));
        return p;
    }
    const float t=ease((flight_-extractTime)/showTime);
    const Vec direction=forward();
    // Match the camera's frame and field of view while keeping the model near the reader.
    float distance=1.85f;
    const float toward=dot(direction,normal);
    if(toward<-.04f)distance=std::min(distance,(dot(eye_-origin,normal)-.72f)/-toward*.62f);
    if(direction.z>.04f)distance=std::min(distance,(roomFront-eye_.z)/direction.z*.73f);
    distance=std::max(.34f,distance);
    const Vec destination=eye_+direction*distance-up()*(distance*.104f);
    p.at=extracted*(1-t)+destination*t;
    const float normalScale=.472f;
    p.scale=p.scale*(1-t)+Vec{normalScale,normalScale,normalScale}*(t*zoom_*distance);
    p.open=flight_==flightEnd?1.f:ease((flight_-openStart)/openingTime);
    p.pitch=(-.09f-.85f*p.open+pitch_)*t;
    const float cameraHeading=-headYaw_;
    const float turn=std::remainder(cameraHeading+yaw_-p.yaw,2*pi);
    p.yaw=t==1.f?yaw_:p.yaw+t*(turn-cameraHeading);
    p.viewYaw=std::remainder(cameraHeading,2*pi)*t;
    p.viewPitch=headPitch_*t;
    const float margin=ease(std::abs(yaw_)*.9f+std::abs(pitch_)*.6f+std::max(0.f,zoom_-1)*.85f)*p.open;
    p.at=p.at+(direction*.035f-up()*.045f)*(distance*margin);
    return p;
}
Mat Library::viewProjection(Size size) const {
    const Vec eye=eye_,fwd=forward(),vertical=up(),horizontal=right();
    const Mat view{{horizontal.x,vertical.x,-fwd.x,0,horizontal.y,vertical.y,-fwd.y,0,
                    horizontal.z,vertical.z,-fwd.z,0,-dot(horizontal,eye),-dot(vertical,eye),dot(fwd,eye),1}};
    const float a=float(size.w)/size.h, f=1/slope(size);
    constexpr float near=.065f,far=65.f;
    const Mat proj{{f/a,0,0,0,0,f,0,0,0,0,(far+near)/(near-far),-1,0,0,2*far*near/(near-far),0}};
    return proj*view;
}
Point Library::project(Vec p,Size size) const {
    const auto q=viewProjection(size).point(p);
    return {(q.x+1)*size.w*.5f,(1-q.y)*size.h*.5f};
}
Ray Library::ray(Point p,Size size) const {
    const float d=slope(size),a=float(size.w)/size.h;
    return {eye_,unit(forward()+right()*((2*p.x/size.w-1)*d*a)+up()*((1-2*p.y/size.h)*d))};
}
int Library::pick(Point p,Size size) const {
    if(stage()!=Stage::Idle || size.w<=0 || size.h<=0)return -1;
    const Ray r=ray(p,size);
    float distance=std::numeric_limits<float>::infinity();int id=-1;
    for(const auto& b:roomOccluders())if(auto t=b.hit(r))distance=std::min(distance,*t);
    if(auto t=Bounds::box(hearthAt+Vec{0,.20f,0},{hearthRadius*2,.40f,hearthRadius*2}).hit(r))
        distance=std::min(distance,*t);
    for(float x:{wingX(0),wingX(1),adultShelfX})
        for(const auto& b:Shelf{x}.solids())if(auto t=b.hit(r))distance=std::min(distance,*t);
    for(const auto& slot:positions()){
        if(bookSection(slot.id)!=section())continue;
        if(auto t=slot.bounds(offsets_[slot.id]).hit(r);t && *t<distance){distance=*t;id=slot.id;}
    }
    return id;
}
bool Library::pickReferences(Point p,Size size) const {
    if(stage()!=Stage::Idle || size.w<=0 || size.h<=0)return false;
    const Ray pointing=ray(p,size);
    const auto hit=referenceLecternBounds().hit(pointing);
    if(!hit || *hit>7.f)return false;
    for(const auto& obstacle:roomOccluders()){
        const auto nearer=obstacle.hit(pointing);
        if(nearer && *nearer<*hit-.001f)return false;
    }
    return true;
}
Ray Library::readerRay(Point p,Size size) const {
    const auto world=ray(p,size);
    const Pose book=pose();
    const auto inv=Mat::scale({1/book.scale.x,1/book.scale.y,1/book.scale.z})*
        Mat::rx(-book.pitch)*Mat::ry(-book.yaw)*Mat::rx(-book.viewPitch)*Mat::ry(-book.viewYaw)*Mat::move(book.at*-1);
    return {inv.point(world.origin),inv.point(world.direction,true)};
}
bool Library::onReader(Point p,Size size) const {
    if(stage()!=Stage::Reading || size.w<=0 || size.h<=0)return false;
    if(selected_==referenceBookId)return page_<1 || referencePage(p,size).has_value();
    return Bounds{{-1.53f,-coverHeight/2,-.06f},{1.53f,coverHeight/2,1.17f}}
        .hit(readerRay(p,size)).has_value();
}
std::optional<Point> Library::referencePage(Point p,Size size) const {
    if(selected_!=referenceBookId || stage()!=Stage::Reading || page_<1 || size.w<=0 || size.h<=0)return {};
    const auto local=readerRay(p,size);
    if(local.direction.z>=-.0001f)return {};
    const float depth=(pageBlock-local.origin.z)/local.direction.z;
    if(depth<=0)return {};
    const Vec hit=local.origin+local.direction*depth;
    if(std::abs(hit.x)<pageInner || std::abs(hit.x)>pageOuter || std::abs(hit.y)>pageHeight/2)return {};
    const float left=hit.x<0?-pageOuter:pageInner;
    return Point{(hit.x-left)/(pageOuter-pageInner)+(hit.x<0?0:1),.5f-hit.y/pageHeight};
}
} // namespace biblioteca
