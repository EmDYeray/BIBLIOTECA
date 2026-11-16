#include "checks.hpp"
#include "desktop.hpp"
#include "popup.hpp"
#include <fontconfig/fontconfig.h>
#include <iostream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <fstream>
#endif
#include <stdexcept>
#include <string_view>

int main(int argc,char** argv){
    try{
        const auto fonts=biblioteca::assetsPath()/"fonts";
        if(auto* config=FcConfigGetCurrent()){
            for(const auto* face:{"Barlow-Regular.ttf","Barlow-Bold.ttf","BarlowCondensed-Bold.ttf"}){
                const auto file=fonts/face;
                if(std::filesystem::is_regular_file(file)){
#ifdef _WIN32
                    const auto utf8=file.u8string();
                    FcConfigAppFontAddFile(config,reinterpret_cast<const FcChar8*>(utf8.c_str()));
#else
                    FcConfigAppFontAddFile(config,reinterpret_cast<const FcChar8*>(file.c_str()));
#endif
                }
            }
        }
        if(argc==1)return biblioteca::interactive();
        const std::string_view option=argv[1];
        if(option=="--self-test"){biblioteca::geometryTest();return 0;}
        if((option=="--render-check" || option=="--desktop-check") && argc==3)
            return biblioteca::renderTest(argv[2],option=="--desktop-check");
        std::ostream& out=option=="--help"?std::cout:std::cerr;
        out << "Biblioteca 3D\n"
               "  biblioteca                       Open the library\n"
               "  biblioteca --self-test           Test geometry and navigation\n"
               "  biblioteca --render-check DIR   Capture frames and test SDL events\n";
        return option=="--help"?0:2;
    }catch(const std::exception& error){
        std::cerr<<"Biblioteca: "<<error.what()<<'\n';
#ifdef _WIN32
        try {
            const auto path=std::filesystem::temp_directory_path()/"Biblioteca3D-error.log";
            std::ofstream log(path,std::ios::app);
            log<<"Biblioteca 3D error: "<<error.what()<<'\n';
            log.flush();
            if(argc==1){
                const std::string message=std::string(error.what())+
                    "\n\nDetails were saved to the Windows temporary folder as Biblioteca3D-error.log.";
                MessageBoxA(nullptr,message.c_str(),"Biblioteca 3D startup error",MB_OK|MB_ICONERROR);
            }
        } catch(...) {}
#endif
        return 1;
    }
}
