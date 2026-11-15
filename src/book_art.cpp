#include "book_art.hpp"
#include "stories.hpp"
#include <initializer_list>

namespace biblioteca {
namespace {
struct Canvas {
    Paint& p;
    Rect panel;
    double x(double t) const {return panel.x+t*panel.w/460;}
    double y(double t) const {return panel.y+t*panel.h/238;}
    void rect(double a,double b,double w,double h,Color c){
        p.fill({x(a),y(b),w*panel.w/460,h*panel.h/238},c);
    }
    void line(double a,double b,double c,double d,Color ink,double weight=2){
        p.line(x(a),y(b),x(c),y(d),ink,weight*panel.w/460);
    }
    void polygon(std::initializer_list<Point> points,Color c){
        cairo_new_path(p.ctx);
        auto v=points.begin();
        cairo_move_to(p.ctx,x(v->x),y(v->y));
        for(++v;v!=points.end();++v)cairo_line_to(p.ctx,x(v->x),y(v->y));
        cairo_close_path(p.ctx);p.ink(c);cairo_fill(p.ctx);
    }
    void disc(double a,double b,double r,Color c){
        cairo_save(p.ctx);
        cairo_translate(p.ctx,x(a),y(b));
        cairo_scale(p.ctx,panel.w/460,panel.h/238);
        p.ink(c);cairo_new_path(p.ctx);cairo_arc(p.ctx,0,0,r,0,2*pi);cairo_fill(p.ctx);
        cairo_restore(p.ctx);
    }
    void label(double a,double b,std::string_view t,Color ink){
        p.font(18*panel.w/460,false,true);p.text(x(a),y(b),t,ink);
    }
};
const auto gold=color("#edbc6d"),silver=color("#e4e5db"),steel=color("#8fa4b3"),
           soil=color("#c0845d"),blue=color("#537caa"),signal=color("#4c9c9d");

void lander(Canvas& c,int page,Illustration model){
    const auto shadow=color("#596278");
    c.polygon({{112,152},{348,152},{385,207},{82,207}},shadow);
    for(double foot:{158.,302.}){
        const double inside=foot<200?196:264;
        c.line(inside,132,foot,182,gold,6);
        c.line(230,150,foot,182,steel,3);
        c.rect(foot-14,180,28,7,gold);
    }
    c.polygon({{186,119},{274,119},{265,153},{195,153}},gold);
    c.polygon({{193,70},{219,51},{249,53},{267,78},{267,124},{194,124}},silver);
    c.rect(214,77,17,15,color("#334662"));c.rect(239,77,13,15,color("#334662"));
    c.line(226,123,226,170,silver,3);c.line(239,123,239,170,silver,3);
    for(int r=0;r<7;++r)c.line(226,128+r*6,239,128+r*6,steel,1.2);
    if(model==Illustration::Falcon){
        c.rect(323,148,53,14,steel);
        for(int i=0;i<3;++i)c.disc(329+i*20,168,8,color("#30374c"));
        c.line(347,149,349,132,gold,3);
    }else if(model==Illustration::Intrepid){
        c.line(343,151,343,173,steel,3);
        c.line(343,161,326,178,steel,3);c.line(343,161,360,178,steel,3);
        c.polygon({{331,136},{356,140},{347,154},{339,153}},silver);
    }
    if(page%4>=2){c.disc(114,145,8,silver);c.line(114,154,109,176,silver,5);c.line(110,169,121,180,steel,4);}
}
void voyager(Canvas& c,Illustration model){
    if(model==Illustration::Kepler){
        c.polygon({{106,109},{187,95},{187,154},{104,166}},blue);
        for(int i=0;i<5;++i)c.line(113+i*14,113,112+i*14,160,steel);
        c.polygon({{178,90},{313,61},{337,137},{203,163}},silver);
        c.disc(320,101,39,steel);c.disc(320,101,32,color("#202941"));c.disc(316,97,23,signal);
        c.line(193,102,207,151,gold,5);
    }else{
        c.disc(342,99,69,color("#d49769"));
        for(int i=-2;i<=2;++i)c.line(289+std::abs(i)*7,98+i*15,395-std::abs(i)*7,98+i*15,color(i%2?"#ebba8f":"#a35448"),6);
        c.disc(363,115,10,color("#ad5846"));
        c.polygon({{96,140},{148,128},{180,153},{157,176},{111,175}},gold);
        c.polygon({{78,86},{200,91},{181,113},{144,123},{109,115}},silver);
        c.line(141,98,145,128,steel,4);
        c.line(148,153,252,134,silver,3);
        c.line(109,159,69,193,steel,3);
    }
}
void mars(Canvas& c,Illustration model){
    c.polygon({{0,167},{57,141},{131,150},{203,137},{279,157},{365,143},{460,158},{460,238},{0,238}},soil);
    c.polygon({{0,201},{161,183},{352,196},{460,182},{460,238},{0,238}},color("#a85845"));
    if(model==Illustration::Insight){
        for(double x:{128.,310.}){
            c.polygon({{float(x-45),137},{float(x-30),110},{float(x+33),111},{float(x+50),143}},blue);
            c.line(x-34,127,x+42,127,steel,2);c.line(x,112,x+6,140,steel,2);
        }
        c.rect(183,115,76,37,gold);
        c.line(190,147,167,183,gold,5);c.line(252,146,266,183,gold,5);
        c.line(229,117,237,81,steel,5);c.line(237,81,282,113,steel,4);
        c.line(282,113,287,151,steel,3);c.disc(286,176,18,silver);
    }else{
        for(int n=0;n<2;++n){
            const double x= n==0?94:265,h=n==0?0:-13;
            for(int i=0;i<3;++i){c.disc(x+22+i*26,173+h,13,color("#30384b"));c.disc(x+22+i*26,173+h,6,steel);}
            c.rect(x,133+h,99,27,silver);c.rect(x+4,127+h,88,9,blue);
            c.line(x+63,134+h,x+60,96+h,steel,5);c.rect(x+44,91+h,36,13,gold);
            c.line(x+93,142+h,x+122,152+h,silver,4);
        }
    }
}
void diagram(Canvas& c,Illustration art,Color ink,Color accent,int page){
    if(art==Illustration::Insight || art==Illustration::Kepler || art==Illustration::Orbit || art==Illustration::Sun){
        const bool transit=art==Illustration::Kepler;
        c.line(70,159,396,159,ink);c.line(70,159,70,49,ink);
        double oldX=75,oldY=96;
        for(int i=1;i<=76;++i){
            const double x=75+i*4.1;
            const double t=i/76.;
            const double v=transit?96+38*ease(float((.18-std::abs(t-.50))/.075))
                :96+(t>.30?std::sin(i*2.11)*37*std::exp(-4.3*(t-.30)):std::sin(i*.84)*2);
            c.line(oldX,oldY,x,v,accent,3);oldX=x;oldY=v;
        }
        c.label(70,194,"Tiempo",ink);
        c.label(83,49,transit?"Brillo relativo":art==Illustration::Insight?"Vibración":"Señal relativa",ink);
        c.label(286,194,"Esquema",ink);
    }else if(art==Illustration::Rovers || art==Illustration::Earth || art==Illustration::Europa){
        c.polygon({{70,99},{157,87},{226,104},{330,78},{395,92},{395,123},{329,109},{227,127},{70,130}},color("#d4a176"));
        c.polygon({{70,130},{227,127},{329,109},{395,123},{395,153},{330,143},{238,155},{70,159}},color("#9c675b"));
        c.rect(70,159,325,25,color("#716278"));
        const double marker=page%4<2?167:289;
        c.line(marker,66,marker,133,ink,3);c.disc(marker,132,5,ink);
        c.label(75,62,art==Illustration::Earth?"Perfil del terreno":art==Illustration::Europa?"Superficie de hielo":"Capas expuestas",ink);
        c.label(72,215,"Esquema de corte",ink);
    }else if(art==Illustration::Saturn || art==Illustration::Pioneer){
        c.disc(216,111,50,color(art==Illustration::Saturn?"#d4ab7c":"#db9368"));
        for(int i=0;i<36;++i){
            const double a=i*2*pi/36,b=(i+1)*2*pi/36;
            c.line(216+116*std::cos(a),111+54*std::sin(a),216+116*std::cos(b),111+54*std::sin(b),accent,2.5);
        }
        c.disc(327,94,7,ink);c.line(330,92,381,59,ink);
        c.label(317,56,"Sonda",ink);c.label(83,194,"Trayectoria esquemática",ink);
    }else{
        if(page==3 || page==5){
            c.polygon({{46,179},{88,153},{151,160},{226,137},{309,160},{390,141},{420,176}},color("#c5b39d"));
            c.rect(93,109,51,43,gold);c.rect(99,117,37,21,blue);
            c.line(120,109,120,73,ink,3);c.line(120,80,144,80,ink,3);
            c.line(143,144,198,161,accent,3);c.disc(200,162,5,ink);
            c.line(257,149,257,99,ink,3);c.line(257,105,282,105,ink,3);
            c.line(288,157,365,157,accent,3);c.disc(368,155,7,ink);
            c.label(83,65,"Estación",ink);c.label(290,112,"Medición",ink);
            c.label(111,208,"Trabajo en la superficie",ink);
            return;
        }
        c.polygon({{171,105},{177,70},{214,41},{244,41},{275,75},{279,105}},silver);
        c.rect(178,116,104,31,color("#c99c59"));
        for(double side:{-1.,1.}){
            c.line(230+side*40,139,230+side*79,179,accent,5);
            c.rect(230+side*79-13,176,26,6,accent);
        }
        c.line(116,88,169,88,ink);c.label(51,76,"Ascenso",ink);
        c.line(284,136,363,136,ink);c.label(311,126,"Descenso",ink);
        c.label(158,211,"Módulo lunar · esquema",ink);
    }
}

bool detail(Canvas& c,Illustration art,int page){
    using V=Illustration;
    if(page%2==0)return false;
    if(art==V::Kepler){
        c.disc(229,117,82,color("#9f645e"));
        c.disc(229,117,66,gold);c.disc(213,101,43,color("#f0cb87"));
        const double x=141+22*page;
        c.line(101,126,360,126,color("#d8b68a",.62),1.3);
        c.disc(x,126,15,color("#28354e"));
        return true;
    }
    if(art==V::Pioneer && page>=5){
        c.rect(92,55,272,138,color("#d9ab63"));
        c.rect(101,64,254,120,color("#b4834f"));
        for(int i=0;i<10;++i){
            const double angle=2*pi*i/10;
            c.line(171,109,171+39*std::cos(angle),109+38*std::sin(angle),color("#ead296"),2);
        }
        c.line(146,158,302,158,color("#ead296"));
        for(int i=0;i<8;++i)c.disc(146+i*22,158,3+i*.28,color("#ead296"));
        for(int i=0;i<2;++i){
            const double x=260+i*28;
            c.disc(x,89,6,color("#ead296"));c.line(x,96,x,126,color("#ead296"),4);
            c.line(x-10,110,x+9,107,color("#ead296"));
        }
        return true;
    }
    if(art==V::Insight && page>=3){
        c.disc(229,130,91,soil);c.disc(229,130,80,color("#a06355"));
        c.disc(229,130,54,color("#df9463"));c.disc(229,130,29,gold);
        c.line(111,77,168,98,silver,2);c.line(349,133,259,131,silver,2);
        c.label(54,62,"Corteza",silver);c.label(352,130,"Núcleo",silver);
        return true;
    }
    if(art==V::Rovers && page>=3){
        c.polygon({{0,129},{120,107},{222,115},{330,100},{460,111},{460,238},{0,238}},soil);
        c.polygon({{0,167},{155,150},{303,164},{460,149},{460,238},{0,238}},color("#9c5448"));
        for(int i=0;i<16;++i)c.disc(66+(i*71)%348,140+(i*39)%69,6+i%5,color("#5f5867"));
        c.line(332,69,295,127,silver,2);c.label(333,64,"Mineral",silver);
        return true;
    }
    if((art==V::Eagle || art==V::Intrepid || art==V::Falcon) && page==7){
        c.disc(253,126,61,color("#5189ac"));c.disc(238,112,39,color("#6ca29a"));
        c.polygon({{211,75},{244,68},{252,96},{230,115},{228,148},{205,127}},color("#a5ba91"));
        c.polygon({{306,102},{299,142},{270,159},{278,131},{278,114}},color("#d9d9ca"));
        c.polygon({{120,149},{151,136},{172,153},{168,176},{135,176}},silver);
        c.line(135,168,105,184,steel,4);
        return true;
    }
    return false;
}
}

Illustration illustrationFor(int book){
    using V=Illustration;
    if(book>=firstImport)return importedBooks()[book-firstImport].art;
    if(book>=originalCount){
        static constexpr std::array original{V::Orbit,V::Orbit,V::Earth,V::Eagle};
        return original[book-originalCount];
    }
    static constexpr std::array byPlace{V::Earth,V::Europa,V::Orbit,V::Rovers,V::Eagle,V::Saturn,V::Sun};
    return byPlace[static_cast<int>(destination(book))];
}

void illustrate(Paint& paint,Rect panel,Illustration art,int page,bool technical){
    using V=Illustration;
    Canvas v{paint,panel};
    if(technical){
        const auto ink=color("#372d30"),accent=color("#855344");
        v.rect(0,0,460,238,color("#e7d8c0"));
        diagram(v,art,ink,accent,page);
        return;
    }
    v.rect(0,0,460,238,color("#28354e"));
    for(int i=0;i<39;++i)
        v.disc((i*137+40+page*13)%448+5,(i*47+8)%155+6,i%8==0?2.5:1.1,color("#f1d2a1",.73));
    if(detail(v,art,page))return;
    if(art==V::Eagle || art==V::Intrepid || art==V::Falcon){
        v.polygon({{0,156},{70,139},{119,149},{236,149},{322,130},{460,151},{460,238},{0,238}},color("#8990a2"));
        v.disc(412,182,29,color("#707b91"));v.disc(57,206,24,color("#707b91"));
        lander(v,page,art);
    }else if(art==V::Insight || art==V::Rovers)mars(v,art);
    else voyager(v,art);
}
}
