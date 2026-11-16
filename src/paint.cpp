#include "paint.hpp"
#include <cstdlib>
#include <sstream>
#include <vector>
#ifdef BIBLIOTECA_IPAD
#include "popup.hpp"
#include <cairo-ft.h>
#include <stdexcept>
#endif

namespace biblioteca {
#ifdef BIBLIOTECA_IPAD
namespace {
struct BundleFont {
    FT_Library library=nullptr;
    FT_Face face=nullptr;
    cairo_font_face_t* cairoFace=nullptr;
    explicit BundleFont(const char* name){
        const auto file=assetsPath()/"fonts"/name;
        if(FT_Init_FreeType(&library) || FT_New_Face(library,file.c_str(),0,&face)){
            if(library)FT_Done_FreeType(library);
            throw std::runtime_error("No se pudo cargar "+file.string());
        }
        cairoFace=cairo_ft_font_face_create_for_ft_face(face,0);
    }
    ~BundleFont(){
        cairo_font_face_destroy(cairoFace);
        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }
};
}
#endif
Color color(std::string_view hex,double alpha) {
    if(hex.starts_with('#'))hex.remove_prefix(1);
    const auto n=std::strtoul(std::string(hex).c_str(),nullptr,16);
    return {double(n>>16&255)/255,double(n>>8&255)/255,double(n&255)/255,alpha};
}
Paint::Paint(Size size):
#ifdef BIBLIOTECA_IPAD
    image(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,size.w,size.h)),
#else
    #ifdef _WIN32
    image(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,size.w,size.h)),
#else
    image(cairo_image_surface_create(0,size.w,size.h)),
#endif
#endif
    ctx(cairo_create(image)),size(size){}
Paint::~Paint(){cairo_destroy(ctx);cairo_surface_destroy(image);}
unsigned char* Paint::pixels(){cairo_surface_flush(image);return cairo_image_surface_get_data(image);}
const unsigned char* Paint::texturePixels() const {
    cairo_surface_flush(image);
    const auto* pixels=cairo_image_surface_get_data(image);
#ifdef BIBLIOTECA_IPAD
    rgba_.assign(pixels,pixels+size_t(size.w)*size.h*4);
    for(size_t i=0;i<rgba_.size();i+=4)std::swap(rgba_[i],rgba_[i+2]);
    return rgba_.data();
#else
    return pixels;
#endif
}
void Paint::ink(Color c){cairo_set_source_rgba(ctx,c.r,c.g,c.b,c.a);}
void Paint::fill(Rect r,Color c){ink(c);cairo_rectangle(ctx,r.x,r.y,r.w,r.h);cairo_fill(ctx);}
void Paint::round(Rect r,double radius,Color inside,Color edge){
    const double x=r.x+r.w,y=r.y+r.h;
    cairo_new_path(ctx);
    cairo_arc(ctx,x-radius,r.y+radius,radius,-pi/2,0);
    cairo_arc(ctx,x-radius,y-radius,radius,0,pi/2);
    cairo_arc(ctx,r.x+radius,y-radius,radius,pi/2,pi);
    cairo_arc(ctx,r.x+radius,r.y+radius,radius,pi,3*pi/2);
    cairo_close_path(ctx);
    ink(inside);cairo_fill_preserve(ctx);ink(edge);cairo_set_line_width(ctx,1);cairo_stroke(ctx);
}
void Paint::line(double x,double y,double x2,double y2,Color c,double weight){
    ink(c);cairo_move_to(ctx,x,y);cairo_line_to(ctx,x2,y2);cairo_set_line_width(ctx,weight);cairo_stroke(ctx);
}
void Paint::diamond(double x,double y,double r,Color c){
    ink(c);cairo_move_to(ctx,x,y-r);cairo_line_to(ctx,x+r,y);cairo_line_to(ctx,x,y+r);
    cairo_line_to(ctx,x-r,y);cairo_close_path(ctx);cairo_set_line_width(ctx,1.5);cairo_stroke(ctx);
}
void Paint::gradient(Rect r,double x1,double y1,double x2,double y2,Color a,Color b){
    auto* pat=cairo_pattern_create_linear(x1,y1,x2,y2);
    cairo_pattern_add_color_stop_rgba(pat,0,a.r,a.g,a.b,a.a);
    cairo_pattern_add_color_stop_rgba(pat,1,b.r,b.g,b.b,b.a);
    cairo_set_source(ctx,pat);cairo_rectangle(ctx,r.x,r.y,r.w,r.h);cairo_fill(ctx);cairo_pattern_destroy(pat);
}
void Paint::font(double px,bool serif,bool bold){
#ifdef BIBLIOTECA_IPAD
    static const BundleFont regular("Barlow-Regular.ttf"),strong("Barlow-Bold.ttf");
    (void)serif;
    cairo_set_font_face(ctx,(bold?strong:regular).cairoFace);
    cairo_set_font_size(ctx,px);
#else
    #ifdef _WIN32
    cairo_select_font_face(ctx,serif?"Liberation Serif":"Barlow",CAIRO_FONT_SLANT_NORMAL,
                           bold?CAIRO_FONT_WEIGHT_BOLD:CAIRO_FONT_WEIGHT_NORMAL);
#else
    cairo_select_font_face(ctx,serif?"Liberation Serif":"Barlow",0,bold?1:0);
#endif
    cairo_set_font_size(ctx,px);
#endif
}
void Paint::display(double px){
#ifdef BIBLIOTECA_IPAD
    static const BundleFont condensed("BarlowCondensed-Bold.ttf");
    cairo_set_font_face(ctx,condensed.cairoFace);
#else
    #ifdef _WIN32
    cairo_select_font_face(ctx,"Barlow Condensed",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);
#else
    cairo_select_font_face(ctx,"Barlow Condensed",0,1);
#endif
#endif
    cairo_set_font_size(ctx,px);
}
double Paint::width(std::string_view t) const {
    cairo_text_extents_t e{};cairo_text_extents(ctx,std::string(t).c_str(),&e);return e.x_advance;
}
double Paint::metricsHeight() const {cairo_font_extents_t e{};cairo_font_extents(ctx,&e);return e.height;}
void Paint::text(double x,double y,std::string_view t,Color c){
    ink(c);cairo_move_to(ctx,x,y);cairo_show_text(ctx,std::string(t).c_str());
}
void Paint::middle(double x,double y,std::string_view t,Color c){text(x-width(t)/2,y,t,c);}
void Paint::tracked(double x,double y,std::string_view t,Color c,double spacing){
    for(size_t i=0;i<t.size();){
        const auto lead=static_cast<unsigned char>(t[i]);
        const size_t n=lead<128?1:lead<224?2:lead<240?3:4;
        const auto glyph=t.substr(i,n);text(x,y,glyph,c);x+=width(glyph)+spacing;i+=n;
    }
}
void Paint::paragraph(Rect r,std::string_view t,Color c,bool centered,double lineSpacing){
    std::istringstream stream{std::string(t)};
    std::vector<std::string> lines;
    std::string word, line;
    while(stream>>word){
        if(!line.empty() && width(line+" "+word)>r.w){lines.push_back(line);line.clear();}
        if(!line.empty())line+=' ';
        line+=word;
    }
    if(!line.empty())lines.push_back(line);
    const double dy=lineSpacing>0?lineSpacing:metricsHeight()*1.12;
    double baseline=r.y+(centered?std::max(0.,(r.h-dy*lines.size())/2):0)+dy*.79;
    for(const auto& l:lines){
        if(baseline>r.y+r.h)break;
        if(centered)middle(r.x+r.w/2,baseline,l,c);else text(r.x,baseline,l,c);
        baseline+=dy;
    }
}
}
