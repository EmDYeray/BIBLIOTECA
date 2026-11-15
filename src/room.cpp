#include "geometry.hpp"
#include "room_layout.hpp"
#include "hearth.hpp"
#include "observatory.hpp"

namespace biblioteca {
namespace {

constexpr std::string_view oak="#97684a",oakEdge="#c99d6d",oakDark="#644630";
constexpr std::string_view plaster="#c9d2bc",darkPlaster="#8aab9e",iron="#20363a",brass="#c1a777";
constexpr std::string_view childPaint="#42787b",childInset="#29434a",adultPaint="#57634a",adultInset="#313e34";
constexpr std::array bindings{"#726777","#a95240","#4f7771","#ba945c","#405b7a",
    "#a67c66","#9a6159","#7b8657","#c5a367","#487680","#d2c6a6","#6b5374"};

void block(MeshData& out,Vec position,Vec size,std::string_view hex,Material mat=Wood,float bevel=.019f){
    box(out,position,size,tint(color(hex)),mat,bevel);
}
unsigned noise(unsigned seed){
    seed=(seed^61)^(seed>>16);
    seed*=9;seed^=seed>>4;seed*=0x27d4eb2d;
    return seed^(seed>>15);
}
float choice(unsigned seed){return float(noise(seed)&1023)/1023.f;}

void volume(MeshData& out,float x,float y,float z,float width,float height,float depth,unsigned id){
    const auto cloth=bindings[noise(id)%bindings.size()];
    constexpr float cover=.018f,spine=.053f;
    const float middle=y+height*.5f;
    block(out,{x,middle,z+(depth-spine)*.5f},{width,height,spine},cloth,Leather,0);
    for(float side:{-1.f,1.f})
        block(out,{x+side*(width-cover)*.5f,middle,z-spine*.5f},
              {cover,height,depth-spine},cloth,Leather,0);
    block(out,{x,middle,z-.022f},{width-2*cover,height-.041f,depth-.060f},"#ad9c82",Paper,0);
    const float front=z+depth/2+.001f;
    if(noise(id+9)%5==0)
        block(out,{x,y+height*.225f,front},{width*.69f,.038f,.0025f},"#c2aa83",Paper,0);
    if(noise(id+15)%6<2)
        for(float band:{.13f,.86f})
            block(out,{x,y+height*band,front},{width*.76f,.009f,.0025f},"#978768",Brass,0);
}

void rank(MeshData& out,float cx,float y,float z,float width,unsigned seed){
    const float right=cx+width/2;
    float x=cx-width/2+.065f;
    for(unsigned i=0;x<right-.15f;++i){
        if(i && (i+seed)%7==0)x+=.20f+choice(i+seed+121)*.24f;
        const float w=.12f+choice(i+seed+7)*.28f;
        if(x+w>right-.055f)break;
        const float h=.50f+choice(i+seed+809)*.36f;
        const float d=.40f+choice(i+seed+91)*.21f;
        volume(out,x+w/2,y,z+(choice(i+seed+52)-.5f)*.10f,w,h,d,i+seed);
        x+=w+.025f+choice(i+seed+43)*.033f;
    }
}
void stack(MeshData& out,Vec at,int offset){
    for(int i=0;i<3;++i){
        const float x=.04f*i,y=.031f+.064f*i,z=.018f*i;
        const float w=.68f-i*.08f,d=.42f-i*.035f;
        block(out,at+Vec{x,y,z},{w,.058f,d},bindings[(offset+i*3)%bindings.size()],Leather,.009f);
        block(out,at+Vec{x,y,z+d*.48f},{w*.94f,.042f,.022f},"#afa18a",Paper,.003f);
    }
}

void fillRow(MeshData& out,float left,float right,float y,float z,
             std::vector<std::pair<float,float>> spaces,unsigned seed){
    std::sort(spaces.begin(),spaces.end());
    for(auto [start,end]:spaces){
        start=std::clamp(start,left,right);
        if(start-left>.29f)rank(out,(start+left)*.5f,y,z,start-left,seed);
        left=std::max(left,std::min(right,end));
        seed+=631;
    }
    if(right-left>.29f)rank(out,(left+right)*.5f,y,z,right-left,seed);
}

void displayCase(MeshData& out,const DisplayCase& item,int index){
    const auto [at,width,wall,seed]=item;
    const float height=caseTop-roomFloor,middle=roomFloor+height*.5f;
    const bool adult=at.x>roomHalfWidth;
    const auto rail=adult?adultPaint:childPaint;
    const auto backing=adult?adultInset:childInset;
    block(out,{0,middle,-.48f},{width,height,.14f},backing,PaintedWood,0);
    for(float x=-width*.5f+.19f;x<width*.5f-.14f;x+=.25f)
        block(out,{x,middle,-.402f},{.016f,height-.45f,.009f},adult?"#394b3c":"#36545a",PaintedWood,0);
    block(out,{0,roomFloor+.080f,0},{width,.16f,1.035f},rail,PaintedWood,.009f);
    for(float sign:{-1.f,1.f}){
        const float edge=sign*(width/2-.083f);
        block(out,{edge,middle,0},{.164f,height,1.04f},rail,PaintedWood);
        block(out,{edge,middle,.5235f},{.066f,height-.20f,.009f},adult?brass:oakEdge,adult?Brass:Wood,0);
    }
    const bool split=splitCase(item);
    const float yaw=displayCaseHeading(wall);
    if(split)block(out,{0,middle,0},{.106f,height,1.03f},rail,PaintedWood);
    for(int row=0;row<6;++row){
        const float y=displayLevel(row);
        block(out,{0,y-.062f,0},{width-.31f,.124f,1.044f},rail,PaintedWood);
        block(out,{0,y-.056f,.5235f},{width-.43f,.021f,.010f},oakEdge,Wood,0);
        std::vector<std::pair<float,float>> gaps;
        for(const auto& slot:positions())if(slot.cabinet==index && slot.row==row){
            const float x=Mat::ry(-yaw).point(slot.at-at,true).x;
            const float trim=books()[slot.id].width*.5f+.085f;
            gaps.emplace_back(x-trim,x+trim);
        }
        if(!gaps.empty()){
            if(split)gaps.emplace_back(-.095f,.095f);
            fillRow(out,-width*.5f+.18f,width*.5f-.19f,y,-.023f,std::move(gaps),seed+row*411);
        }else if(split){
            const float bay=width*.5f-.28f,x=width*.25f+.05f;
            rank(out,-x,y,-.035f,bay,seed+row*411);
            rank(out,x,y,-.009f,bay,seed+row*411+301);
        }else{
            const float offset=row%3==1 && wall!=CaseWall::Stories?.79f:0;
            rank(out,-offset*.5f,y,-.025f,width-.40f-offset,seed+row*411);
            if(offset>0)stack(out,{width*.5f-.565f,y,.011f},int(seed%6)+row);
        }
    }
    block(out,{0,caseTop-.067f,0},{width,.134f,1.05f},rail,PaintedWood);
    const Mat orient=Mat::move(at)*Mat::ry(yaw);
    for(auto& v:out){v.p=orient.point(v.p);v.n=orient.point(v.n,true);}
}

void primary(MeshData& out,int wing){
    const bool adult=wing==2;
    const Shelf shelf{adult?adultShelfX:wingX(wing)};
    const auto solids=shelf.solids();
    const auto rail=adult?adultPaint:childPaint;
    const auto backing=adult?adultInset:childInset;
    for(size_t i=0;i<solids.size();++i)
        block(out,(solids[i].min+solids[i].max)/2,solids[i].max-solids[i].min,
              i?rail:backing,PaintedWood,.025f);
    auto add=[&](Vec p,Vec s,std::string_view c,Material mat=PaintedWood){
        p.x+=shelf.x;block(out,p,s,c,mat);
    };
    add({0,-2.191f,.028f},{8.64f,.130f,1.22f},backing);
    add({0,3.37f,-.532f},{8.26f,2.25f,.18f},backing);
    add({0,4.43f,.008f},{8.48f,.148f,1.14f},rail);
    add({0,3.287f,.018f},{8.14f,.106f,1.11f},rail);
    for(float y:{-2.20f,-.781f,.648f,2.038f,3.27f,4.439f})
        add({0,y,y<2.1f?.754f:.585f},{8.02f,.043f,.017f},adult?oak:oakEdge,Wood);
    for(float x:{-4.11f,4.11f})
        add({x,3.27f,.025f},{.270f,2.47f,1.17f},rail);
    chartRelief(out,{shelf.x,5.33f,-.24f},adult,wing);
    for(int row=0;row<2;++row){
        const float y=2.302f+row*1.038f;
        const float sign=(wing==row?-1.f:1.f);
        rank(out,shelf.x+sign*1.86f,y,-.002f,3.84f,2401+352*wing+129*row);
        rank(out,shelf.x-sign*2.27f,y,-.042f,2.80f,3421+702*wing+154*row);
        if(row==0)stack(out,{shelf.x-sign*.42f,y,.02f},wing*4);
    }
    for(int row=0;row<3;++row){
        const float y=Shelf::levels[row];
        std::vector<std::pair<float,float>> gaps;
        for(const auto& slot:positions())
            if(slot.cabinet<0 && slot.row==row && std::abs(slot.at.x-shelf.x)<4.f){
                const float trim=books()[slot.id].width*.5f+.085f;
                gaps.emplace_back(slot.at.x-trim,slot.at.x+trim);
            }
        fillRow(out,shelf.x-3.89f,shelf.x+3.89f,y,.006f,std::move(gaps),6437+row*711+wing*329);
    }
}

void ladder(MeshData& out,float x){
    constexpr float height=5.50f;
    const auto lean=Mat::move({x,roomFloor+.006f,1.42f})*Mat::rx(-.11f);
    MeshData local;
    for(float side:{-.312f,.312f})
        block(local,{side,height/2,0},{.076f,height,.107f},oakDark,Wood,.014f);
    for(int i=0;i<10;++i)block(local,{0,.36f+i*.50f,0},{.601f,.064f,.150f},oakEdge,Wood,.012f);
    for(auto v:local){v.p=lean.point(v.p);v.n=lean.point(v.n,true);out.push_back(v);}
}

void table(MeshData& out,Vec at,bool stocked){
    const bool adult=at.x>roomHalfWidth;
    block(out,at+Vec{0,.973f,0},{3.08f,.112f,1.52f},adult?oak:oakEdge,Wood,.045f);
    block(out,at+Vec{0,1.031f,0},{2.55f,.008f,.93f},adult?"#674a3d":"#567677",Leather,.003f);
    block(out,at+Vec{0,.903f,0},{2.92f,.041f,1.37f},brass,Brass,.013f);
    for(float x:{-1.26f,1.26f}){
        for(float z:{-.464f,.464f}){
            block(out,at+Vec{x,.478f,z},{.133f,.901f,.135f},oakDark,Wood,.026f);
            block(out,at+Vec{x,.059f,z},{.143f,.040f,.147f},iron,Metal,.012f);
        }
        block(out,at+Vec{x,.357f,0},{.091f,.096f,1.06f},oakDark,Wood,.014f);
    }
    block(out,at+Vec{0,.375f,0},{2.58f,.082f,.128f},oakDark,Wood,.014f);
    if(stocked){
        stack(out,at+Vec{-.79f,1.031f,-.21f},3);
        block(out,at+Vec{.80f,1.037f,-.25f},{.528f,.036f,.356f},"#577873",Leather,.012f);
        block(out,at+Vec{.80f,1.056f,-.25f},{.474f,.011f,.318f},"#aeb4a1",Paper,.009f);
    }
}
void chair(MeshData& out,Vec at,float rear){
    for(float x:{-.334f,.334f})for(float z:{-.282f,.282f})
        block(out,at+Vec{x,.277f,z},{.078f,.545f,.084f},oakDark,Wood,.014f);
    block(out,at+Vec{0,.535f,0},{.803f,.096f,.696f},oak,Wood,.031f);
    block(out,at+Vec{0,.594f,0},{.741f,.064f,.626f},at.x>roomHalfWidth?"#7b694c":"#386f70",Textile,.023f);
    for(float x:{-.331f,.331f})
        block(out,at+Vec{x,.889f,rear*.348f},{.073f,.664f,.084f},oakDark,Wood,.012f);
    block(out,at+Vec{0,1.099f,rear*.35f},{.746f,.158f,.089f},oak,Wood,.032f);
    block(out,at+Vec{0,.916f,rear*.345f},{.653f,.060f,.071f},oakDark,Wood,.018f);
    for(float x:{-.218f,0.f,.218f})
        block(out,at+Vec{x,1.015f,rear*.346f},{.081f,.154f,.053f},oakEdge,Wood,.012f);
}
void bench(MeshData& out,Vec at){
    block(out,at+Vec{0,.52f,0},{4.48f,.18f,1.06f},oak);
    block(out,at+Vec{0,.630f,0},{4.37f,.069f,1.0f},"#557b8e",Textile,.032f);
    for(float x:{-1.77f,1.77f})
        block(out,at+Vec{x,.228f,0},{.228f,.464f,.861f},oakDark);
}

void floor(MeshData& out,float width,float depth,float center,float centerX=0){
    const float run=centerX>roomHalfWidth?.56f:.94f;
    constexpr float gap=.010f;
    constexpr std::array birch{"#bba386","#c6ae8e","#c3a787","#c6af96","#c4a986",
        "#bfa68a","#c5af95","#c2a98d","#cbb393"};
    constexpr std::array walnut{"#806451","#91745c","#826b58","#94725c","#836553",
        "#8b7059","#987861","#8d6c55","#8c705b"};
    const auto& shades=centerX>roomHalfWidth?walnut:birch;
    block(out,{centerX,roomFloor-.126f,center},{width+.11f,.241f,depth+.13f},"#36525c",Floor,0);
    for(int i=0;float(i)*run<depth;++i){
        const float start=roomBack+float(i)*run;
        const float row=std::min(run,roomFront-start);
        float x=-width/2;
        float length=1.18f+choice(i+947)*3.7f;
        for(int j=0;x<width/2-.02f;++j){
            const float q=std::min(length,width/2-x);
            block(out,{centerX+x+q/2,roomFloor-.027f,start+row/2},{q-gap,.054f,row-gap},
                  shades[noise(i*47+j*189)%shades.size()],Floor,0);
            x+=q;length=3.66f+choice(i*23+j*471+197)*1.97f;
        }
    }
}

void shell(MeshData& out){
    const float width=2*roomHalfWidth,depth=roomFront-roomBack;
    const float z=(roomBack+roomFront)/2,middle=(roomFloor+roomCeiling)/2,h=roomCeiling-roomFloor;
    floor(out,width,depth,z);
    block(out,{0,roomCeiling+.10f,z},{width+.28f,.20f,depth+.28f},"#ddd7bd",Wall,0);
    for(float x:{-11.f,-4.f,4.f,11.f})
        block(out,{x,roomCeiling-.11f,z},{.14f,.23f,depth-.18f},oakEdge,Wood,0);
    block(out,{0,middle,roomBack-.10f},{width+.28f,h,.20f},darkPlaster,Wall,0);
    block(out,{0,middle,roomFront+.10f},{width+.28f,h,.20f},plaster,Wall,0);
    for(float sign:{-1.f,1.f}){
        const float x=sign*roomHalfWidth;
        if(sign<0){
            block(out,{x+sign*.10f,middle,z},{.20f,h,depth},plaster,Wall,0);
            block(out,{x-sign*.029f,roomFloor+.12f,z},{.065f,.24f,depth},oakDark,Wood,.008f);
        }
        block(out,{x-sign*.22f,roomCeiling-.60f,z},{.47f,.16f,depth},oakDark,Wood,.015f);
        // A recessed frosted strip sends light up into the cove.
        block(out,{x-sign*.263f,roomCeiling-.508f,z},{.10f,.018f,depth-.30f},"#d7b88f",LightStrip,.003f);
    }
    for(const auto& wall:dividerWalls())
        block(out,(wall.min+wall.max)*.5f,wall.max-wall.min,plaster,Wall,0);
    for(float side:{-1.f,1.f}){
        const float jamb=doorZ+side*(doorHalf-.038f);
        block(out,{roomHalfWidth,(roomFloor+1.63f)*.5f,jamb},{.48f,1.63f-roomFloor,.085f},adultInset,PaintedWood,.008f);
        block(out,{roomHalfWidth+.025f,(roomFloor+1.51f)*.5f,jamb+side*.10f},
              {.225f,1.51f-roomFloor,.127f},brass,Brass,.014f);
    }
    block(out,{roomHalfWidth,1.585f,doorZ},{.49f,.148f,2*doorHalf+.16f},adultInset,PaintedWood);
    block(out,{roomHalfWidth,1.955f,doorZ},{.26f,.35f,2.48f},adultInset,PaintedWood,.008f);
    const auto leaf=openDoorLeaf();
    const Vec leafCenter=(leaf.min+leaf.max)*.5f;
    block(out,leafCenter,leaf.max-leaf.min,adultInset,PaintedWood,.016f);
    for(float face:{-1.f,1.f}){
        const float faceZ=leafCenter.z+face*.060f;
        block(out,{leafCenter.x,.42f,faceZ},{2.26f,1.56f,.024f},adultPaint,PaintedWood,.013f);
        block(out,{leafCenter.x,-1.20f,faceZ},{2.26f,1.46f,.024f},adultPaint,PaintedWood,.013f);
        block(out,{leaf.max.x-.16f,-.55f,faceZ+face*.036f},{.101f,.24f,.066f},"#8c714e",Metal,.008f);
        block(out,{leaf.max.x-.275f,-.526f,faceZ+face*.066f},{.256f,.035f,.046f},"#b48f63",Metal,.007f);
    }
    block(out,{roomHalfWidth,roomFloor+.006f,doorZ},{.48f,.012f,2*doorHalf-.08f},"#695750",Metal,.004f);
    block(out,{roomHalfWidth-.254f,1.51f,doorZ},{.023f,.022f,2*doorHalf-.34f},"#cfa97e",LightStrip,.004f);
    block(out,{0,roomFloor+.12f,roomFront-.030f},{width,.24f,.066f},oakDark,Wood,.008f);
    block(out,{0,roomCeiling-.60f,roomFront-.207f},{width,.16f,.45f},oakDark);
    block(out,{0,roomCeiling-.507f,roomFront-.259f},{width-.42f,.018f,.10f},"#d7b88f",LightStrip,.003f);
    block(out,{0,roomCeiling-.60f,roomBack+.206f},{width,.16f,.45f},oakDark);
    // Shadow gap above the continuous bookcase line.
    block(out,{0,4.694f,-.65f},{width-.24f,.228f,1.32f},oakDark);
    block(out,{0,4.812f,-.585f},{width-.33f,.013f,.091f},"#e4bd93",LightStrip,0);
    for(auto [kind,at,face]:furniture()){
        if(at.x>roomHalfWidth)continue;
        if(kind==Furnishing::Table)table(out,at,at.x<0);
        else if(kind==Furnishing::Chair)chair(out,at,face);
        else bench(out,at);
    }
    hearthInlay(out);
    buildHearth(out);
    ceilingInstrument(out);
}

void adultRoom(MeshData& out){
    const float width=adultRight-roomHalfWidth, depth=roomFront-roomBack;
    const float x=(adultRight+roomHalfWidth)*.5f,z=(roomBack+roomFront)*.5f;
    const float y=(roomFloor+roomCeiling)*.5f,h=roomCeiling-roomFloor;
    floor(out,width,depth,z,x);
    block(out,{x,roomCeiling+.10f,z},{width+.08f,.20f,depth+.28f},"#a3a895",Wall,0);
    block(out,{x,y,roomBack-.10f},{width+.09f,h,.20f},"#818873",Wall,0);
    block(out,{x,y,roomFront+.10f},{width+.09f,h,.20f},"#92927a",Wall,0);
    block(out,{adultRight+.10f,y,z},{.20f,h,depth},"#a29e85",Wall,0);
    for(float wall:{roomHalfWidth+.24f,adultRight-.24f}){
        block(out,{wall,roomCeiling-.60f,z},{.44f,.16f,depth},oakDark);
        block(out,{wall,roomCeiling-.509f,z},{.09f,.018f,depth-.28f},"#c1a383",LightStrip,.003f);
    }
    for(float wall:{roomBack+.20f,roomFront-.20f}){
        block(out,{x,roomCeiling-.60f,wall},{width,.16f,.44f},oakDark);
        block(out,{x,roomCeiling-.509f,wall},{width-.25f,.018f,.112f},"#c1a383",LightStrip,.003f);
    }
    for(const auto& item:furniture()){
        if(item.at.x<roomHalfWidth)continue;
        if(item.kind==Furnishing::Table)table(out,item.at,true);
        else chair(out,item.at,item.face);
    }
    for(const auto& at:lanterns)pendant(out,at);
}
}

std::array<RoomSection,roomSectionCount> room(){
    std::array<RoomSection,roomSectionCount> sections;
    shell(sections[0].mesh);
    const auto& cases=displayCases();
    for(size_t i=0;i<cases.size();++i)displayCase(sections[i+1].mesh,cases[i],int(i));
    primary(sections[sections.size()-4].mesh,0);
    primary(sections[sections.size()-3].mesh,1);
    ladder(sections[sections.size()-3].mesh,wingX(1)+3.67f);
    primary(sections[sections.size()-2].mesh,2);
    adultRoom(sections.back().mesh);
    return sections;
}
}
