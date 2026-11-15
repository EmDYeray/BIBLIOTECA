#include "geometry.hpp"
namespace biblioteca {
Vec tint(Color c){return {float(c.r),float(c.g),float(c.b)};}
void quad(MeshData& out,const std::array<Vertex,4>& q){for(int i:{0,1,2,0,2,3})out.push_back(q[i]);}
void box(MeshData& out,Vec center,Vec extent,Vec color,Material mat,float bevel,int printAxis,int printSide){
    const Vec r=extent/2;
    if(bevel<=0){
        for(int axis=0;axis<3;++axis){
            const int u=(axis+1)%3,v=(axis+2)%3;
            for(float side:{-1.f,1.f}){
                auto vertex=[&](float x,float y){
                    std::array<float,3> point{},normal{};
                    point[axis]=r[axis]*side;point[u]=r[u]*x;point[v]=r[v]*y;
                    normal[axis]=side;
                    return Vertex{center+Vec{point[0],point[1],point[2]},
                        {normal[0],normal[1],normal[2]},color,0,0,float(mat)};
                };
                const auto a=vertex(-1,-1),b=vertex(1,-1),c=vertex(1,1),d=vertex(-1,1);
                if(side>0)quad(out,{a,b,c,d});else quad(out,{d,c,b,a});
            }
        }
        return;
    }
    const float cut=std::min({bevel,r.x*.47f,r.y*.47f,r.z*.47f});
    const Vec core=r-Vec{cut,cut,cut};
    for(int axis=0;axis<3;++axis){
        const int u=(axis+1)%3,v=(axis+2)%3;
        const std::array<float,4> us{-r[u],-core[u],core[u],r[u]},vs{-r[v],-core[v],core[v],r[v]};
        for(float side:{-1.f,1.f}){
            auto vertex=[&](int i,int j){
                std::array<float,3> p{},a{};
                p[axis]=r[axis]*side;p[u]=us[i];p[v]=vs[j];
                for(int k=0;k<3;++k)a[k]=std::clamp(p[k],-core[k],core[k]);
                const Vec base{a[0],a[1],a[2]},n=unit(Vec{p[0],p[1],p[2]}-base);
                const Vec pos=base+n*cut;
                const bool print=axis==printAxis && side==printSide;
                const float sx=axis==0?pos.z:pos.x;
                const float sw=axis==0?extent.z:extent.x;
                return Vertex{center+pos,n,color,.5f+sx/sw,.5f-pos.y/extent.y,float(print?printedFront:int(mat))};
            };
            for(int j=0;j<3;++j)for(int i=0;i<3;++i){
                const auto a=vertex(i,j),b=vertex(i+1,j),c=vertex(i+1,j+1),d=vertex(i,j+1);
                if(side>0)quad(out,{a,b,c,d});else quad(out,{d,c,b,a});
            }
        }
    }
}

}
