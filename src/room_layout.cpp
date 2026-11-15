#include "room_layout.hpp"
#include <queue>

namespace biblioteca {

const std::array<DisplayCase,displayCaseCount>& displayCases(){
    static const auto cases=[]{
        std::array<DisplayCase,displayCaseCount> result{{
            {{0,0,-.62f},1.59f,CaseWall::Stories,9531},
            {{-13.17f,0,-.62f},6.72f,CaseWall::Stories,3419},
            {{13.17f,0,-.62f},6.72f,CaseWall::Stories,7764}
        }};
        constexpr float gap=.028f;
        constexpr float wallInset=caseDepth*.5f+.013f;
        constexpr float sideStart=-.050f,sideEnd=roomFront-caseDepth-.036f;
        constexpr float sideBay=(sideEnd-sideStart)/sideCaseCount;
        size_t cursor=3;
        auto bay=[&](float x,float z,float width,CaseWall wall){
            result[cursor]={{x,0,z},width-gap,wall,11051u+unsigned(cursor)*791u};
            ++cursor;
        };
        for(size_t i=0;i<sideCaseCount;++i)
            bay(-roomHalfWidth+wallInset,sideStart+(float(i)+.5f)*sideBay,sideBay,CaseWall::Left);
        const std::array sides{std::array{sideStart,doorZ-doorHalf-.29f},
                               std::array{doorZ+doorHalf+.29f,sideEnd}};
        for(const auto [a,b]:sides)
            for(int i=0;i<2;++i)bay(roomHalfWidth-wallInset,(a+b)*.5f+(i?1:-1)*(b-a)*.25f,
                                   (b-a)*.5f,CaseWall::Right);
        constexpr float rearHalf=roomHalfWidth-caseDepth-.043f;
        constexpr float rearBay=2*rearHalf/rearCaseCount;
        for(size_t i=0;i<rearCaseCount;++i){
            const float x=-rearHalf+(float(i)+.5f)*rearBay;
            bay(x,roomFront-wallInset,rearBay,CaseWall::Rear);
        }
        bay(19.51f,-.62f,4.1f,CaseWall::Stories);
        bay(34.23f,-.62f,4.25f,CaseWall::Stories);
        constexpr float adultFirst=roomHalfWidth+caseDepth+.15f,adultLast=adultRight-caseDepth-.06f;
        const float adultRear=(adultLast-adultFirst)/3.f;
        for(int i=0;i<3;++i){
            bay(adultFirst+(i+.5f)*adultRear,roomFront-wallInset,adultRear,CaseWall::Rear);
            const float depth=(sideEnd-sideStart)/3.f;
            bay(adultRight-wallInset,sideStart+(i+.5f)*depth,depth,CaseWall::Right);
        }
        for(const auto [a,b]:sides)
            bay(roomHalfWidth+wallInset+.12f,(a+b)*.5f,b-a,CaseWall::Left);
        return result;
    }();
    return cases;
}

Bounds displayCaseBounds(const DisplayCase& item){
    const bool side=item.wall==CaseWall::Left || item.wall==CaseWall::Right;
    const Vec extent{side?caseDepth:item.width,caseTop-roomFloor,side?item.width:caseDepth};
    return Bounds::box({item.at.x,(roomFloor+caseTop)*.5f,item.at.z},extent);
}
float displayCaseHeading(CaseWall wall){
    return wall==CaseWall::Left?pi*.5f:wall==CaseWall::Right?-pi*.5f:wall==CaseWall::Rear?pi:0;
}
bool splitCase(const DisplayCase& item){return item.width>4.45f && item.wall==CaseWall::Rear;}
std::vector<Bounds> displayCaseBoards(const DisplayCase& item){
    const bool side=item.wall==CaseWall::Left || item.wall==CaseWall::Right;
    const auto transform=Mat::move(item.at)*Mat::ry(displayCaseHeading(item.wall));
    const float height=caseTop-roomFloor,middle=roomFloor+height*.5f;
    const float w=item.width;
    std::vector<Bounds> solids;
    const auto board=[&](Vec p,Vec s){
        solids.push_back(Bounds::box(transform.point(p),side?Vec{s.z,s.y,s.x}:s));
    };
    board({0,middle,-.48f},{w,height,.14f});
    board({0,roomFloor+.080f,0},{w,.16f,1.035f});
    for(float sign:{-1.f,1.f})board({sign*(w/2-.083f),middle,0},{.164f,height,1.04f});
    if(splitCase(item))board({0,middle,0},{.106f,height,1.03f});
    for(int row=0;row<6;++row)board({0,displayLevel(row)-.062f,0},{w-.31f,.124f,1.05f});
    board({0,caseTop-.067f,0},{w,.134f,1.05f});
    return solids;
}

const std::array<Furniture,furnitureCount>& furniture(){
    static constexpr std::array<Furniture,furnitureCount> pieces{{
        {Furnishing::Table,{-7.40f,roomFloor,8.0f}},
        {Furnishing::Chair,{-7.40f,roomFloor,6.37f},-1},
        {Furnishing::Chair,{-7.40f,roomFloor,9.63f}},
        {Furnishing::Table,{5.85f,roomFloor,9.55f}},
        {Furnishing::Chair,{5.85f,roomFloor,7.92f},-1},
        {Furnishing::Chair,{5.85f,roomFloor,11.18f}},
        {Furnishing::Bench,{-8.75f,roomFloor,11.96f}},
        {Furnishing::Bench,{8.75f,roomFloor,11.96f}},
        {Furnishing::Table,{23.3f,roomFloor,6.5f}},
        {Furnishing::Chair,{23.3f,roomFloor,4.89f},-1},
        {Furnishing::Chair,{23.3f,roomFloor,8.11f}},
        {Furnishing::Table,{30.67f,roomFloor,7.85f}},
        {Furnishing::Chair,{30.67f,roomFloor,6.24f},-1},
        {Furnishing::Chair,{30.67f,roomFloor,9.46f}}
    }};
    return pieces;
}

Bounds furnitureBounds(const Furniture& item){
    const Vec size=item.kind==Furnishing::Table?Vec{3.08f,1.03f,1.52f}
        :item.kind==Furnishing::Chair?Vec{.91f,1.27f,.83f}:Vec{4.48f,.66f,1.06f};
    return Bounds::box(item.at+Vec{0,size.y/2,0},size);
}

std::array<Bounds,3> dividerWalls(){
    constexpr float bottom=doorZ-doorHalf,top=doorZ+doorHalf;
    const auto wall=[](float low,float high,float y0,float y1){
        return Bounds::box({roomHalfWidth,(y0+y1)*.5f,(low+high)*.5f},{.22f,y1-y0,high-low});
    };
    return {wall(roomBack,bottom,roomFloor,roomCeiling),wall(top,roomFront,roomFloor,roomCeiling),
            wall(bottom,top,1.57f,roomCeiling)};
}
Bounds openDoorLeaf(){
    return Bounds::box({roomHalfWidth+1.38f,(roomFloor+1.48f)*.5f+.03f,doorZ+doorHalf-.084f},
                       {2.66f,1.48f-roomFloor-.06f,.112f});
}

Bounds referenceLecternBounds(){
    return Bounds::box(referenceLecternAt+Vec{0,.82f,0},{.71f,1.64f,1.15f});
}

const std::vector<Bounds>& roomOccluders(){
    static const auto geometry=[]{
        std::vector<Bounds> result;
        for(const auto& item:furniture())result.push_back(furnitureBounds(item));
        result.push_back(referenceLecternBounds());
        for(const auto& item:displayCases()){
            const auto boards=displayCaseBoards(item);
            result.insert(result.end(),boards.begin(),boards.end());
        }
        result.push_back(Bounds::box({wingX(1)+3.67f,.55f,1.12f},{.77f,5.61f,.84f}));
        for(const auto& wall:dividerWalls())result.push_back(wall);
        result.push_back(openDoorLeaf());
        for(float sign:{-1.f,1.f})
            result.push_back(Bounds::box({roomHalfWidth,(roomFloor+1.63f)*.5f,doorZ+sign*(doorHalf-.038f)},
                                         {.48f,1.63f-roomFloor,.085f}));
        return result;
    }();
    return geometry;
}

bool walkable(Vec p){
    if(p.x < -roomHalfWidth+personRadius || p.x > adultRight-personRadius ||
       p.z < roomBack+personRadius || p.z > roomFront-personRadius)return false;
    const float dx=p.x-hearthAt.x,dz=p.z-hearthAt.z;
    if(dx*dx+dz*dz<(hearthRadius+personRadius)*(hearthRadius+personRadius))return false;
    static const auto solids=[]{
        auto result=roomOccluders();
        for(const auto& item:displayCases())result.push_back(displayCaseBounds(item));
        // Leave space at the front for a book to clear the shelf before it turns.
        for(int i=0;i<2;++i)result.push_back(Bounds::box({wingX(i),1.3f,.24f},{8.92f,7.1f,2.07f}));
        result.push_back(Bounds::box({adultShelfX,1.3f,.24f},{8.92f,7.1f,2.07f}));
        return result;
    }();
    for(const auto& b:solids)
        if(b.min.y<=walkingEye+.2f && p.x>b.min.x-personRadius && p.x<b.max.x+personRadius &&
           p.z>b.min.z-personRadius && p.z<b.max.z+personRadius)return false;
    return true;
}

Vec walkStep(Vec start,Vec delta){
    const int steps=std::max(1,int(std::ceil(std::max(std::abs(delta.x),std::abs(delta.z))/.085f)));
    delta=delta/float(steps);
    for(int i=0;i<steps;++i){
        const Vec x=start+Vec{delta.x,0,0};
        if(walkable(x))start=x;
        const Vec z=start+Vec{0,0,delta.z};
        if(walkable(z))start=z;
    }
    return start;
}

std::vector<Vec> routeInside(Vec from,Vec to){
    auto direct=[](Vec a,Vec b){
        const Vec delta=b-a;
        const int steps=std::max(1,int(std::ceil(std::hypot(delta.x,delta.z)/.075f)));
        for(int i=1;i<=steps;++i)if(!walkable(a+delta*(float(i)/steps)))return false;
        return true;
    };
    if(direct(from,to))return {to};
    constexpr int nx=107,nz=34;
    constexpr float cell=.5f,xMin=-16.5f,zMin=-1.85f;
    auto point=[](int k){return Vec{xMin+cell*(k%nx),walkingEye,zMin+cell*(k/nx)};};
    auto nearest=[&](Vec p){
        int id=-1;float best=4;
        for(int k=0;k<nx*nz;++k){
            const Vec q=point(k);const float dist=dot(p-q,p-q);
            if(dist<best && walkable(q) && direct(p,q)){best=dist;id=k;}
        }
        return id;
    };
    const int start=nearest(from),finish=nearest(to);
    if(start<0 || finish<0)return {};
    std::array<int,nx*nz> previous;previous.fill(-1);
    std::queue<int> next;next.push(start);previous[start]=start;
    while(!next.empty() && previous[finish]<0){
        const int k=next.front();next.pop();
        for(auto [dx,dz]:std::array<std::array<int,2>,4>{{{1,0},{-1,0},{0,1},{0,-1}}}){
            const int x=k%nx+dx,z=k/nx+dz;
            if(x<0 || x>=nx || z<0 || z>=nz)continue;
            const int j=z*nx+x;
            if(previous[j]<0 && walkable(point(j)) && direct(point(k),point(j))){
                previous[j]=k;next.push(j);
            }
        }
    }
    if(previous[finish]<0)return {};
    std::vector<Vec> path{to};
    for(int k=finish;;k=previous[k]){
        path.push_back(point(k));
        if(k==start)break;
    }
    std::vector<Vec> result;
    while(!path.empty()){
        size_t i=0;
        while(i+1<path.size() && !direct(from,path[i]))++i;
        from=path[i];result.push_back(from);path.resize(i);
    }
    return result;
}
}
