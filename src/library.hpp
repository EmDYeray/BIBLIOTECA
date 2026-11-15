#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace biblioteca {

constexpr float pi=3.14159265358979323846f;
struct Vec {
    float x=0,y=0,z=0;
    float operator[](int i) const { return i==0 ? x : i==1 ? y : z; }
    Vec operator+(Vec b) const { return {x+b.x,y+b.y,z+b.z}; }
    Vec operator-(Vec b) const { return {x-b.x,y-b.y,z-b.z}; }
    Vec operator*(float k) const { return {x*k,y*k,z*k}; }
    Vec operator/(float k) const { return *this*(1/k); }
};
inline float dot(Vec a,Vec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline Vec cross(Vec a,Vec b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
inline Vec unit(Vec v) { return v/std::sqrt(dot(v,v)); }
struct Mat {
    std::array<float,16> a{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    static Mat move(Vec v);
    static Mat scale(Vec v);
    static Mat rx(float theta);
    static Mat ry(float theta);
    Mat operator*(const Mat& other) const;
    Vec point(Vec p,bool direction=false) const;
};
struct Point { float x=0,y=0; };
struct Size { int w=0,h=0; };
struct Ray { Vec origin,direction; };
struct Bounds {
    Vec min,max;
    static Bounds box(Vec center,Vec size);
    bool overlaps(const Bounds& b) const;
    std::optional<float> hit(Ray r) const;
};
struct Book {
    std::string title,genre,description,cover,foil;
    float width,height;
    int year;
    std::string number(int id) const;
};
enum class Section { Children,Adult };
constexpr std::string_view sectionName(Section room){
    return room==Section::Children?"Sala infantil":"Sala juvenil y adulta";
}
constexpr int originalCount=42,originalAdultCount=4,importedChildren=8,importedAdults=5;
constexpr int firstImport=originalCount+originalAdultCount,bookCount=firstImport+importedChildren+importedAdults;
constexpr std::array technicalOriginals{9,14,15,16,18,25,26,36,37,41};
constexpr int childrenCount=originalCount-int(technicalOriginals.size())+importedChildren;
constexpr int adultCount=bookCount-childrenCount;
constexpr int spreads=8,adultSpreads=4;
constexpr int referenceBookId=bookCount;
constexpr Section bookSection(int id){
    if(id<originalCount)
        return std::find(technicalOriginals.begin(),technicalOriginals.end(),id)==technicalOriginals.end()
            ?Section::Children:Section::Adult;
    return id>=firstImport && id<firstImport+importedChildren?Section::Children:Section::Adult;
}
constexpr bool panoramicBook(int id){return id<originalCount && bookSection(id)==Section::Children;}
constexpr int spreadCount(int id){return id==referenceBookId?2:panoramicBook(id)?spreads:adultSpreads;}
const std::array<Book,bookCount>& books();
std::string pageText(int id,int page);
struct Slot {
    int id,wing,row;
    Vec at;
    int cabinet=-1;
    float heading=0;
    float height() const;
    Vec outward() const;
    Vec spine() const;
    Vec approachPoint() const;
    Bounds bounds(float push=0) const;
};
const std::array<Slot,bookCount>& positions();
struct Shelf {
    float x;
    static constexpr std::array<float,3> levels{-2.10f,-.68f,.74f};
    static constexpr float top=2.09f, front=.757f;
    std::array<Bounds,7> solids() const;
};
constexpr float pageBlock=.077f, hingeGap=.016f, board=.027f, coverHeight=1.42f;
constexpr float pageInner=.035f, pageOuter=.967f, pageHeight=1.337f, spineEdge=.035f;
constexpr float closedZ=2*(pageBlock+hingeGap), closedThickness=closedZ+2*board;
constexpr float shelfDepth=.88f, slotZ=.08f, extractedZ=1.40f;
constexpr float extractTime=.36f, showTime=.52f, openingTime=.66f;
constexpr float openStart=extractTime+showTime, flightEnd=openStart+openingTime, turnDuration=1.25f;
constexpr float launchDuration=2.10f, trailFadeDuration=2.10f;
constexpr float launchSequenceDuration=launchDuration+trailFadeDuration;

float wingX(int wing);
float ease(float x);
struct Pose { Vec at,scale; float yaw,pitch,open; float viewYaw=0,viewPitch=0; Mat model() const; };
Pose onShelf(int id,float push=0);
Pose onLectern();
struct Panels { Mat rear,front; float pivotZ,xOffset; };
Panels panels(float opening);
Vec leafPoint(float radial,float y,float t);
enum class Stage { Idle,Traveling,Extracting,Presenting,Opening,Reading,Closing,Returning };

class Library {
    int wing_=0, selected_=-1, hovering_=-1, queued_=-1;
    int spread_=0, to_=0, lecternSpread_=0;
    Vec eye_{-1.14f,-.46f,13.13f};
    float headYaw_=.13f,headPitch_=-.019f;
    std::vector<Vec> route_;
    size_t waypoint_=0;
    Vec aim_{};
    float flight_=0, popupClock_=0, page_=1, settle_=0, pickedPush_=0;
    bool motionEnabled_=true;
    float yaw_=0,pitch_=0,zoom_=1,goalYaw_=0,goalPitch_=0,goalZoom_=1;
    bool return_=false;
    bool extract(int id);
    void leaveLectern();
    std::array<float,bookCount> offsets_{};
public:
    bool wingTo(int wing);
    Section section() const;
    bool enter(Section room);
    bool visit(int id);
    void look(float dx,float dy);
    void walk(float forward,float side);
    Vec eye() const {return eye_;}
    Vec forward() const;
    Vec right() const;
    Vec up() const;
    bool travel(Vec to,Vec aim,int book=-1);
    bool open(int id);
    bool openReferences();
    void close();
    bool turn(int direction);
    void update(float dt);
    void hover(int id);
    void orbit(float dx,float dy);
    void zoom(float steps);
    float push(int id) const {return offsets_[id];}
    int wing() const {return wing_;}
    int selected() const {return selected_;}
    int hovered() const {return hovering_;}
    int spread() const {return spread_;}
    int lecternSpread() const {return lecternSpread_;}
    int target() const {return to_;}
    float turning() const {return page_;}
    float popupClock() const {return popupClock_;}
    void setMotionEnabled(bool enabled){motionEnabled_=enabled;}
    bool animating() const;
    Stage stage() const;
    Pose pose() const;
    Mat viewProjection(Size size) const;
    Point project(Vec p,Size size) const;
    Ray ray(Point p,Size size) const;
    int pick(Point p,Size size) const;
    bool pickReferences(Point p,Size size) const;
    bool onReader(Point p,Size size) const;
    std::optional<Point> referencePage(Point p,Size size) const;
private:
    Ray readerRay(Point p,Size size) const;
};
} // namespace biblioteca
