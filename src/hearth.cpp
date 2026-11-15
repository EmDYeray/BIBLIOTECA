#include "hearth.hpp"
#include "room_layout.hpp"

namespace biblioteca {
namespace {
struct Turn {float radius,height;};
void band(MeshData& mesh,Vec center,Turn from,Turn to,Vec pigment,Material material,int sides=64){
    auto vertex=[&](int step,Turn point){
        const float angle=2*pi*step/sides,c=std::cos(angle),s=std::sin(angle);
        return Vertex{center+Vec{point.radius*c,point.height,point.radius*s},
                      unit(Vec{(to.height-from.height)*c,from.radius-to.radius,(to.height-from.height)*s}),
                      pigment,0,0,float(material)};
    };
    for(int i=0;i<sides;++i)quad(mesh,{vertex(i,from),vertex(i,to),vertex(i+1,to),vertex(i+1,from)});
}
void disk(MeshData& mesh,Vec at,float radius,Vec pigment,Material material,int sides=48){
    const auto v=[&](Vec p){return Vertex{at+p,{0,1,0},pigment,0,0,float(material)};};
    for(int i=0;i<sides;++i){
        const float a=2*pi*i/sides,b=2*pi*(i+1)/sides;
        mesh.insert(mesh.end(),{v({0,0,0}),v({radius*std::cos(b),0,radius*std::sin(b)}),
                               v({radius*std::cos(a),0,radius*std::sin(a)})});
    }
}
void log(MeshData& mesh,Vec at,float len,float radius,float angle,int seed){
    constexpr int sides=9;
    const Vec axis{std::cos(angle),0,std::sin(angle)};
    const Vec side{-std::sin(angle),0,std::cos(angle)};
    auto rim=[&](int i,float end){
        const float a=2*pi*i/sides;
        const Vec n=Vec{0,std::cos(a),0}+side*std::sin(a);
        const float d=radius*(1.f+.10f*std::sin(i*4.47f+seed));
        return Vertex{at+axis*(end*len/2)+n*d,n,tint(color(i%4==0?"#49352c":"#302725")),0,0,Wood};
    };
    for(int i=0;i<sides;++i){
        quad(mesh,{rim(i,-1),rim(i+1,-1),rim(i+1,1),rim(i,1)});
        for(float sign:{-1.f,1.f}){
            auto a=rim(i,sign),b=rim(i+1,sign);
            const Vec pigment=tint(color(i%3==0?"#724938":"#4d352b"));
            a.n=b.n=axis*sign;a.c=b.c=pigment;
            const Vertex center{at+axis*(sign*len/2),axis*sign,pigment,0,0,Wood};
            if(sign>0)mesh.insert(mesh.end(),{center,a,b});
            else mesh.insert(mesh.end(),{center,b,a});
        }
    }
}
}

void buildHearth(MeshData& out){
    const Vec stone=tint(color("#76a39f")),rim=tint(color("#326a6b"));
    const Vec iron=tint(color("#9a714e")),inside=tint(color("#2a2323"));
    // The landing sits flush with the floor. Only the raised ring stops the camera.
    band(out,hearthAt,{1.67f,.005f},{0,.009f},tint(color("#315363")),Tile);
    band(out,hearthAt,{1.68f,.008f},{1.65f,.013f},tint(color("#ac8c62")),Brass);
    band(out,hearthAt,{hearthRadius-.08f,.014f},{hearthRadius,.098f},rim,Tile);
    band(out,hearthAt,{hearthRadius,.098f},{hearthRadius,.285f},rim,Tile);
    band(out,hearthAt,{hearthRadius+.001f,.105f},{hearthRadius+.001f,.135f},tint(color("#a9c6b3")),Tile);
    band(out,hearthAt,{hearthRadius,.285f},{hearthRadius-.061f,.335f},stone,Tile);
    band(out,hearthAt,{hearthRadius-.061f,.335f},{.956f,.335f},stone,Tile);
    band(out,hearthAt,{.956f,.335f},{.908f,.312f},iron,Metal);
    band(out,hearthAt,{.908f,.312f},{.816f,.175f},inside,Wall);
    disk(out,hearthAt+Vec{0,.175f,0},.817f,inside,Wall);

    log(out,hearthAt+Vec{-.020f,.250f,.256f},1.15f,.089f,.17f,7);
    log(out,hearthAt+Vec{.096f,.248f,-.227f},1.18f,.10f,-.23f,13);
    log(out,hearthAt+Vec{-.232f,.316f,-.033f},1.04f,.076f,1.01f,31);
    log(out,hearthAt+Vec{.196f,.290f,.028f},.95f,.077f,2.42f,41);
    for(int i=0;i<25;++i){
        const float angle=i*2.39996f,r=.18f+.49f*std::sqrt(float(i)/25);
        const Vec at=hearthAt+Vec{r*std::cos(angle),.185f,r*std::sin(angle)};
        const float size=.027f+.052f*(i%7)/7.f;
        const Vec pigment=tint(color(i%3==0?"#d65f38":"#854333"));
        band(out,at,{size,0},{size*.91f,.029f},pigment,Ember,7);
        band(out,at,{size*.91f,.029f},{size*.46f,.049f},pigment,Ember,7);
        disk(out,at+Vec{0,.049f,0},size*.46f,pigment,Ember,7);
    }

    const Vec hood=tint(color("#25434f")),edge=tint(color("#c49f67"));
    band(out,hearthAt,{1.115f,3.430f},{1.156f,3.462f},edge,Brass);
    band(out,hearthAt,{1.156f,3.462f},{1.125f,3.512f},hood,Metal);
    band(out,hearthAt,{1.125f,3.512f},{.884f,3.691f},hood,Metal);
    band(out,hearthAt,{.884f,3.691f},{.235f,4.143f},hood,Metal);
    band(out,hearthAt,{.235f,4.143f},{.202f,4.21f},hood,Metal);
    band(out,hearthAt,{.202f,4.21f},{.202f,roomCeiling-roomFloor},hood,Metal);
    band(out,hearthAt,{.98f,3.477f},{1.115f,3.430f},inside,Metal);
    band(out,hearthAt,{.18f,3.65f},{.98f,3.477f},inside,Metal);
}

MeshData hearthFlames(){
    MeshData mesh;
    constexpr int sides=14,heightSteps=19;
    struct Tongue {Vec at;float radius,height,lean,phase,strength;};
    constexpr std::array<Tongue,8> tongues{{
        {{-.29f,.294f,-.096f},.202f,.98f,-.20f,1.8f,.84f},
        {{ .17f,.312f, .18f},.193f,1.15f,.18f,3.7f,.84f},
        {{-.03f,.332f,-.11f},.230f,1.51f,.13f,2.9f,1.f},
        {{ .35f,.267f,-.090f},.156f,.87f,-.10f,4.6f,.68f},
        {{-.35f,.269f, .244f},.134f,.66f,.16f,5.9f,.76f},
        {{-.07f,.328f, .315f},.137f,.86f,-.08f,.3f,.91f},
        {{ .17f,.318f,-.265f},.165f,1.11f,.15f,6.6f,.82f},
        {{ .01f,.312f, .013f},.170f,.85f,-.07f,7.8f,.94f}
    }};
    for(const auto [at,radius,height,lean,phase,strength]:tongues){
        auto vertex=[&](int i,int j){
            const float t=float(j)/heightSteps,a=2*pi*i/sides;
            const float r=radius*(.62f+1.07f*t)*std::pow(1.f-t,1.37f)+.002f;
            const Vec normal{std::cos(a),.12f*t,std::sin(a)};
            const Vec p=at+Vec{lean*t*t+normal.x*r,t*height,.037f*std::sin(t*6+phase)*t+normal.z*r};
            return Vertex{p,normal,{strength,height,0},t,phase,Ember};
        };
        for(int j=0;j<heightSteps;++j)for(int i=0;i<sides;++i)
            quad(mesh,{vertex(i,j),vertex(i,j+1),vertex(i+1,j+1),vertex(i+1,j)});
    }
    return mesh;
}
}
