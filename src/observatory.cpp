#include "observatory.hpp"

namespace biblioteca {
namespace {
constexpr int detail=80;
const Vec copper=tint(color("#cfaf74")),patina=tint(color("#346861")),
          porcelain=tint(color("#f2e4c2")),blue=tint(color("#237477")),
          night=tint(color("#20535c"));

Vec polar(float a,float radius,float height){
    return {radius*std::cos(a),height,radius*std::sin(a)};
}

void profile(MeshData& out,Vec at,Vec a,Vec b,Vec pigment,Material material){
    const auto tip=[&](int i,Vec p){
        const float theta=i*2*pi/detail;
        const auto n=unit(polar(theta,b.y-a.y,a.x-b.x));
        return Vertex{at+polar(theta,p.x,p.y),n,pigment,0,0,float(material)};
    };
    for(int i=0;i<detail;++i)
        quad(out,{tip(i,a),tip(i,b),tip(i+1,b),tip(i+1,a)});
}

void ring(MeshData& out,Vec at,Vec e,Vec f,float radius,float thickness,Vec pigment,Material material,int steps=detail){
    const int corners=steps<50?4:6;
    const Vec axis=cross(e,f);
    auto node=[&](int i,int j){
        const float a=i*2*pi/steps,b=j*2*pi/corners;
        const Vec radial=e*std::cos(a)+f*std::sin(a);
        const Vec n=radial*std::cos(b)+axis*std::sin(b);
        return Vertex{at+radial*radius+n*thickness,n,pigment,0,0,float(material)};
    };
    for(int i=0;i<steps;++i)for(int j=0;j<corners;++j)
        quad(out,{node(i,j),node(i+1,j),node(i+1,j+1),node(i,j+1)});
}

void sector(MeshData& out,float a,float b,float inner,float outer,Vec pigment,Material kind,float rise){
    const auto v=[&](float angle,float r){
        return Vertex{hearthAt+polar(angle,r,rise),{0,1,0},pigment,0,0,float(kind)};
    };
    quad(out,{v(a,inner),v(b,inner),v(b,outer),v(a,outer)});
}
}

void hearthInlay(MeshData& out){
    for(int i=0;i<detail;++i){
        const float a=i*2*pi/detail,b=(i+1)*2*pi/detail;
        sector(out,a+.002f,b-.002f,1.67f,3.26f,i%10<6?blue:night,Tile,.003f);
        sector(out,a,b,1.70f,1.74f,copper,Brass,.007f);
        sector(out,a,b,2.13f,2.156f,porcelain,Tile,.007f);
        sector(out,a,b,3.13f,3.16f,copper,Brass,.007f);
        if(i%5==0){
            const float end=i%10==0?3.05f:2.88f;
            sector(out,a-.018f,a+.018f,2.72f,end,porcelain,Tile,.009f);
        }
    }
    for(int i=0;i<8;++i){
        const float a=i*2*pi/8;
        for(int j=0;j<5;++j){
            const float width=.011f*(5-j);
            sector(out,a-width,a+width,2.33f+j*.08f,2.41f+j*.08f,copper,Brass,.009f);
        }
    }
}

void ceilingInstrument(MeshData& out){
    const Vec at=hearthAt+Vec{0,4.48f,0};
    ring(out,at,{1,0,0},{0,0,1},1.60f,.047f,copper,Brass);
    ring(out,at,{1,0,0},{0,.66f,.7513f},1.49f,.033f,patina,Metal);
    ring(out,at,{.7986f,0,.6018f},{-.399f,.748f,.530f},1.35f,.026f,porcelain,Tile);
    for(int i=0;i<12;++i){
        const auto position=at+polar(i*2*pi/12,1.60f,0);
        box(out,position,{.082f,.062f,.082f},patina,Metal,.022f);
    }
    for(float x:{-.65f,.65f})
        box(out,{at.x+x,(at.y+roomCeiling)*.5f,at.z+.18f},
            {.024f,roomCeiling-at.y-.023f,.024f},patina,Metal,.004f);
}

void pendant(MeshData& out,Vec at){
    const Vec shade=tint(color("#536244")),trim=tint(color("#d7ac77"));
    const float stem=roomCeiling-at.y;
    profile(out,at,{.036f,.30f},{.036f,stem},trim,Brass);
    profile(out,at,{.42f,-.19f},{.48f,-.13f},trim,Brass);
    profile(out,at,{.48f,-.13f},{.437f,-.02f},shade,PaintedWood);
    profile(out,at,{.437f,-.02f},{.33f,.16f},shade,PaintedWood);
    profile(out,at,{.33f,.16f},{.14f,.26f},shade,PaintedWood);
    profile(out,at,{.14f,.26f},{.061f,.30f},trim,Brass);
    profile(out,at,{.03f,-.36f},{.24f,-.34f},porcelain,Frosted);
    profile(out,at,{.24f,-.34f},{.35f,-.27f},porcelain,Frosted);
    profile(out,at,{.35f,-.27f},{.42f,-.19f},porcelain,Frosted);
}

void chartRelief(MeshData& out,Vec at,bool technical,int layout){
    const auto frame=tint(color(technical?"#603e2b":"#c9a574"));
    const auto backing=tint(color(technical?"#263c33":"#244c52"));
    const float width=technical?7.f:6.7f;
    box(out,at,{width,1.01f,.085f},frame,Wood,.035f);
    box(out,at+Vec{0,0,.044f},{width-.16f,.85f,.025f},backing,PaintedWood,.012f);
    for(float rail:{-.318f,.318f})
        box(out,at+Vec{0,rail,.063f},{width-.36f,.019f,.016f},copper,Brass,0);
    for(int i=0;i<3;++i){
        const float dx=(i-1)*2.03f;
        const auto center=at+Vec{dx,.012f,.068f};
        ring(out,center,{1,0,0},{0,1,0},.238f,.012f,porcelain,Tile,32);
        const float angle=(layout+i)*.46f;
        const Vec e{std::cos(angle),std::sin(angle),0};
        const Vec f{-e.y*.52f,e.x*.52f,.8542f};
        ring(out,center+Vec{0,0,.082f},e,f,.292f,.009f,copper,Brass,32);
        box(out,center+e*.18f+Vec{0,0,.078f},{.053f,.052f,.026f},copper,Brass,.008f);
    }
}
}
